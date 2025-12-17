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

std::tuple<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t> calculate_ellpack_matVec_metrics(
    tt::tt_metal::IDevice* device,
    const tt::daisy::tt_ldu_meta& tt_meta,
    EllpackHwImpl hwImpl
) {
    if (!tt_meta.ellpack_addr_ || tt_meta.cell_count == 0) {
        throw std::runtime_error("tt_ldu_meta does not have ellpack_addr_ set");
    }

    uint32_t ell_tile_page_size = tt_metal::detail::TileSize(tt::DataFormat::Float32);
    uint32_t ell_tiles_total = (tt_meta.cell_count + 31u) / 32u;
    uint32_t tiles_in_batch = hwImpl == EllpackHwImpl::FPU? 4u : 8u;

    uint64_t dram_bytes_wr = ell_tiles_total * 32 * sizeof(float); // multiples of 32 floats in result
    uint32_t vecs_per_page = vector_block_size/sizeof(float);
    uint32_t vector_page_size = vector_block_size;
    auto vec2page_shift = static_cast<uint32_t>(std::log2(vecs_per_page));

    auto avail_cores = device->compute_with_storage_grid_size();

    auto [num_cores, used_cores, core_group_1, core_group_2, work_per_core1, work_per_core2] =
        tt::tt_metal::split_work_to_cores(avail_cores, ell_tiles_total);

    uint64_t dram_bytes_rd = ell_tile_page_size*2 * ell_tiles_total; // mat + addr

    auto first_tile = 0u;
    for (int c = 0; c < num_cores; ++c) {
        auto tiles = c < core_group_1.num_cores() ? work_per_core1 : work_per_core2;
        auto end_tile = first_tile + tiles;
        for (uint32_t t = first_tile; t < end_tile; t += tiles_in_batch) {
            auto last_t = std::min(end_tile, t + tiles_in_batch) -1;
            auto in_batch = last_t + 1 - t;
            auto first_vec = tt_meta.ellpack_first_col_per_tile_[t];
            auto last_vec = tt_meta.ellpack_last_col_per_tile_[last_t];
            auto first_vec_page = first_vec >> vec2page_shift;
            auto last_vec_page = last_vec >> vec2page_shift;
            auto vec_pages = last_vec_page - first_vec_page + 1;
            dram_bytes_rd += vec_pages * vector_page_size;
            // std::cout << " dram_rd " << vec_pages << " vpages for t" << t << "..+" << in_batch << " on c" << c << std::endl;
        }
        first_tile += tiles;
    }

    // flops calculated:
    uint64_t mul_flops = hwImpl == EllpackHwImpl::FPU?
        ell_tiles_total * 32 * 32 * 32
        : tt_meta.ellpack_avg_cols_ * tt_meta.cell_count;
    uint64_t add_flops = hwImpl == EllpackHwImpl::FPU?
        ell_tiles_total * (32 * 32 * 32 -1)
        : std::max(tt_meta.ellpack_avg_cols_-1, 0.0f) * tt_meta.cell_count;
    

    uint64_t ellpack_mat_bytes = ell_tiles_total * ell_tile_page_size;
    uint64_t bare_vector_size = ell_tiles_total * 32 * sizeof(float);


    
    return {
        dram_bytes_rd,
        dram_bytes_wr,
        mul_flops,
        add_flops,
        ellpack_mat_bytes,
        bare_vector_size
    };
}

void tt_launch_ellpack_matVecOp(
    tt::tt_metal::IDevice* device,
    tt::daisy::tt_ldu_meta& tt_meta,
    tt::tt_metal::Buffer& d_inVec,
    tt::tt_metal::Buffer& d_resVec,
    const std::filesystem::path& kernel_dir,
    EllpackHwImpl hwImpl,
    size_t region_id
) {
    // static int invocation = 0;

    // std::cout << "Launching Ellpack MatVec Op (invocation " << invocation++ << ") with hwImpl " << static_cast<int>(hwImpl) << std::endl;

    const bool diag_wb = hwImpl == EllpackHwImpl::FPU;

    tt::tt_metal::Program program;
    // assume 1 tile wide ellpack (in allocation)
    auto ell_used_cols = tt_meta.ellpack_cols_;
    auto ell_tiles_total = (tt_meta.cell_count + 31u) / 32u;
    auto data_format = tt::DataFormat::Float32;
    uint32_t ell_tile_page_size = tt_metal::detail::TileSize(data_format);
    uint32_t vecs_per_page = vector_block_size/sizeof(float);
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
            __daisy_instrumentation_increment(region_id, "tt_used_cores", num_cores);
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
    tt_metal::TensorAccessorArgs(tt_meta.d_ellpack_vals_).append_to(rd_compile_args, rd_common_args);
    tt_metal::TensorAccessorArgs(tt_meta.d_ellpack_addrs_).append_to(rd_compile_args, rd_common_args);
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
            tt_meta.d_ellpack_vals_->address(),
            tt_meta.d_ellpack_addrs_->address(),
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

            auto first_vec = tt_meta.ellpack_first_col_per_tile_[start_tile];
            auto last_vec  = tt_meta.ellpack_last_col_per_tile_[start_tile + tiles -1];

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
