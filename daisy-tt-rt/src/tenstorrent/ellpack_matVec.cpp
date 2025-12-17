#include "ellpack_matVec.hpp"
#include "hostdevcommon/kernel_structs.h"
#include "tt-metalium/buffer.hpp"
#include "tt-metalium/tt_backend_api_types.hpp"

#include <cmath>
#include <cstdint>
#include <tt-metalium/host_api.hpp>
#include <tt-metalium/work_split.hpp>
#include <filesystem>
#include <tt-metalium/kernel_types.hpp>
#include <tt-metalium/tensor_accessor_args.hpp>
#include <tt-metalium/tt_metal.hpp>

#ifdef ENABLE_DAISY_RTL
#include <daisy_rtl/daisy_rtl.h>
#endif

namespace tt::daisy {

#define TT_DEBUG 0

void tt_launch_ellpack_matVecOp(
    tt::tt_metal::IDevice* device,
    uint32_t cells,
    uint32_t ellpack_cols,
    tt::tt_metal::Buffer& d_ellpack_vals,
    tt::tt_metal::Buffer& d_ellpack_addrs,
    tt::tt_metal::Buffer& d_inVec,
    tt::tt_metal::Buffer& d_resVec,
    uint32_t* ellpack_first_col_per_tile,
    uint32_t* ellpack_last_col_per_tile,
    const std::filesystem::path& kernel_dir,
    EllpackHwImpl hwImpl,
    size_t region_id
) {
    // static int invocation = 0;

    // std::cout << "Launching Ellpack MatVec Op (invocation " << invocation++ << ") with hwImpl " << static_cast<int>(hwImpl) << std::endl;

    const bool diag_wb = hwImpl == EllpackHwImpl::FPU;

    tt::tt_metal::Program program;
    // assume 1 tile wide ellpack (in allocation)
    auto ell_used_cols = ellpack_cols;
    auto ell_tiles_total = (cells + 31u) / 32u;
    auto data_format = tt::DataFormat::Float32;
    uint32_t ell_tile_page_size = tt_metal::detail::TileSize(data_format);
    uint32_t vecs_per_page = d_inVec.page_size()/sizeof(float);
    auto vec_page2tile_shift = 2u;
    auto vec2page_shift = static_cast<uint32_t>(std::log2(vecs_per_page));
    auto vecs_per_chunk = 1024u;
    auto vec_chunk2page_shift = 3u;
    auto vec_chunk_size = vecs_per_chunk * sizeof(float);

    uint32_t vector_page_size = vecs_per_page*sizeof(float);
    uint32_t vec_per_tile = 32u;

    uint32_t max_tile_batch_size = diag_wb? 4u : 8u;

    auto avail_cores = device->compute_with_storage_grid_size();

    auto [num_cores, used_cores, core_group_1, core_group_2, work_per_core1, work_per_core2] =
        tt::tt_metal::split_work_to_cores(avail_cores, ell_tiles_total);

    #ifdef ENABLE_DAISY_RTL
        if (region_id != 0) {
            auto num_avail_cores = static_cast<double>(avail_cores.x * avail_cores.y);
            auto num_cores_d = static_cast<double>(num_cores);
            __daisy_instrumentation_metric(region_id, "tt_used_cores", num_cores_d);
            __daisy_instrumentation_metric(region_id, "tt_cores_used_rel", num_cores_d / num_avail_cores);
            __daisy_instrumentation_metric(region_id, "tt_work_units_per_core", ell_tiles_total / num_avail_cores);
        }
    #endif

    #if TT_DEBUG > 0
    std::cout << "Using " << num_cores << " cores to process " << ell_tiles_total << " tiles, " << max_tile_batch_size << " tiles max. per batch ("
              << work_per_core1 << " on " << core_group_1.num_cores() << ", " << work_per_core2 << " on " << core_group_2.num_cores() << "; )" << std::endl;
    #endif

    auto input_tile_count = max_tile_batch_size * 2;
    auto result_page_count = 4;

    // c0 output (vector)
    // c1 input (mat - ellpack data)
    // c2 input (mat - ellpack addr)
    // c3 input (vector)
    // c4 temp (mat - mulmat of vector matched to ellpack tiles)
    tt_metal::CreateCircularBuffer(
        program,
        used_cores,  // create on all cores
        tt_metal::CircularBufferConfig(
            ell_tile_page_size * input_tile_count,
            {
                {CBIndex::c_1, data_format},
            }
        )
        .set_page_size(CBIndex::c_1, ell_tile_page_size)
    );

    tt_metal::CreateCircularBuffer(
        program,
        used_cores,  // create on all cores
        tt_metal::CircularBufferConfig(
            ell_tile_page_size * input_tile_count,
            {
                {CBIndex::c_4, data_format},
            }
        )
        .set_page_size(CBIndex::c_4, ell_tile_page_size)
    );

    tt_metal::CreateCircularBuffer(
        program,
        used_cores,  // create on all cores
        tt_metal::CircularBufferConfig(
            ell_tile_page_size * input_tile_count,
            {
                {CBIndex::c_2, tt::DataFormat::Float32} // we MUST lie, because only then will tt-runtime unlock TF32 support (the host runtime maps the "dest" type, which is used for DST and SrcA/SrcB regs, Tf32 cannot be manually set, but must be chosed as Dest type to not loose precision)
                // tensix driver will use the dest type as is for unpack byte size. And will use it as is for SrcA/SrcB input type (which if FP32 will expect FP16 and therefore misinterpret the unpacked data)
                // CBs throw, if we try to use them with TF32 explicitly, because there is a switch case that does not define a byte-size
                // default mapping will either map FP16 as dest type (by default) or FP32 (if UnpackToDestFp32 is set. No other effects on the host side)
            }
        )
        .set_page_size(CBIndex::c_2, ell_tile_page_size));

    auto res_buf_page_size = (diag_wb? tt_metal::detail::TileSize(data_format) : (vec_per_tile * sizeof(float)));
    tt_metal::CreateCircularBuffer(
        program,
        used_cores,  // create on all cores
        tt_metal::CircularBufferConfig(
            res_buf_page_size * max_tile_batch_size * 2,
            {
                {CBIndex::c_0, data_format},
            }
        )
        .set_page_size(CBIndex::c_0, res_buf_page_size)
    );

    tt_metal::CreateCircularBuffer(
        program,
        used_cores,  // create on all cores
        tt_metal::CircularBufferConfig(
            vec_chunk_size * 2,
            {
                {CBIndex::c_3, data_format}
            }
        )
        .set_page_size(CBIndex::c_3, vec_chunk_size)
    );

    std::vector<uint32_t> rd_compile_args, rd_common_args;
    tt_metal::TensorAccessorArgs(d_ellpack_vals).append_to(rd_compile_args, rd_common_args);
    tt_metal::TensorAccessorArgs(d_ellpack_addrs).append_to(rd_compile_args, rd_common_args);
    tt_metal::TensorAccessorArgs(d_inVec).append_to(rd_compile_args, rd_common_args);
    auto kernel_rd_0 = tt_metal::CreateKernel(
        program,
        kernel_dir / "ellpack" / "mat_vec_reader_naive.cpp",
        used_cores,
        tt_metal::ReaderDataMovementConfig(
            rd_compile_args
        )
    );

    std::vector<uint32_t> wr_compile_args, wr_common_args;
    wr_compile_args.push_back(diag_wb? 1u : 0u); // unpack_diag
    tt_metal::TensorAccessorArgs(d_resVec).append_to(wr_compile_args, wr_common_args);
    auto kernel_wr_0 = tt_metal::CreateKernel(
        program,
        kernel_dir / "ellpack" / "vec32_result_wb.cpp", // depends on compile_time arg if it does diag's job
        used_cores,
        tt_metal::WriterDataMovementConfig(
            wr_compile_args
        )
    );

    std::vector<UnpackToDestMode> unpack_modes(NUM_CIRCULAR_BUFFERS, UnpackToDestMode::Default);
    // unpack_modes[CBIndex::c_1] = UnpackToDestMode::UnpackToDestFp32;
    // unpack_modes[CBIndex::c_4] = UnpackToDestMode::UnpackToDestFp32;

    auto kernel_comp_0 = tt_metal::CreateKernel(
        program,
        kernel_dir / "ellpack" / (diag_wb? "mat_vec_compute_matmul.cpp" : "mat_vec_compute_naive.cpp"),
        used_cores,
        tt_metal::ComputeConfig {
            .math_fidelity = MathFidelity::HiFi4,
            .fp32_dest_acc_en = true,
            // .dst_full_sync_en = false,
            // .unpack_to_dest_mode = unpack_modes,
            // .math_approx_mode = false,
            .compile_args = {},
        }
    );

    rd_common_args.insert(
        rd_common_args.begin(),
        {
            d_ellpack_vals.address(),
            d_ellpack_addrs.address(),
            d_inVec.address(),
            max_tile_batch_size,
            vecs_per_page,
            vec2page_shift,
            vec_chunk2page_shift
        }
    );

    tt_metal::SetCommonRuntimeArgs(
        program,
        kernel_rd_0,
        rd_common_args
    );

    tt_metal::SetCommonRuntimeArgs(
        program,
        kernel_comp_0,
        {
            max_tile_batch_size,
            vecs_per_chunk,
        }
    );

    wr_common_args.insert(
        wr_common_args.begin(),
        {
            d_resVec.address(),
            vecs_per_page,
            vec_page2tile_shift
        }
    );

    tt_metal::SetCommonRuntimeArgs(
        program,
        kernel_wr_0,
        wr_common_args
    );


    uint32_t start_tile = 0;
    uint32_t end_tile = ell_tiles_total; // ex

    for (auto& range : used_cores.ranges()) {

        for (auto& core : range) {
            uint32_t tiles;
            if (core_group_1.contains(core)) {
                tiles = work_per_core1;
            } else if (core_group_2.contains(core)) {
                tiles = work_per_core2;
            } else {
                tiles = 0;
            }

            if (start_tile + tiles > end_tile) {
                tiles = end_tile - start_tile;
            }

            auto first_vec = ellpack_first_col_per_tile[start_tile];
            auto last_vec  = ellpack_last_col_per_tile[start_tile + tiles -1];

            #if TT_DEBUG >= 2
            std::cout << " Core " << core.str() << ": tiles " << start_tile << "..+" << tiles << ", vec " << first_vec << ".." << last_vec << std::endl;
            #endif

            tt::tt_metal::SetRuntimeArgs(
                program,
                kernel_rd_0,
                core,
                {
                    start_tile,
                    tiles,
                    first_vec,
                    last_vec
                }
            );

            tt::tt_metal::SetRuntimeArgs(
                program,
                kernel_comp_0,
                core,
                {
                    tiles,
                    first_vec,
                    last_vec
                }
            );

            tt::tt_metal::SetRuntimeArgs(
                program,
                kernel_wr_0,
                core,
                {
                    start_tile,
                    tiles,
                }
            );

            start_tile += tiles;
        }
    }

    tt_metal::EnqueueProgram(device->command_queue(0), program, false);
}

}   // namespace tt::daisy::foam
