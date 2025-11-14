#include "ellpack_matVec.hpp"
#include "hostdevcommon/kernel_structs.h"
#include "tt-metalium/buffer.hpp"
#include "tt-metalium/tt_backend_api_types.hpp"

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
    const tt::daisy::tt_ldu_meta& tt_meta,
    EllpackHwImpl hwImpl
) {
    if (!tt_meta.ellpack_addr_ || tt_meta.cell_count == 0) {
        throw std::runtime_error("tt_ldu_meta does not have ellpack_addr_ set");
    }

    uint32_t vector_page_size = 1024u;
    uint32_t ell_tile_page_size = tt_metal::detail::TileSize(tt::DataFormat::Float32);
    uint32_t ell_tiles_total = (tt_meta.cell_count + 31u) / 32u;

    uint32_t batch_size = hwImpl == EllpackHwImpl::FPU? 4u : 8u;
    auto batches_total = (ell_tiles_total + batch_size - 1) / batch_size;

    uint64_t dram_bytes_rd = batches_total * ell_tiles_total * 32 * sizeof(float) + ell_tiles_total * ell_tile_page_size;
    uint64_t dram_bytes_wr = hwImpl == EllpackHwImpl::FPU ?
        ell_tiles_total * 32 * sizeof(float) // multiples of 32 floats in result
        : batches_total * vector_page_size; // multiples of 1024 bytes in result

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

    const bool diag_wb = hwImpl == EllpackHwImpl::FPU;

    tt::tt_metal::Program program;
    // assume 1 tile wide ellpack (in allocation)
    auto ell_used_cols = tt_meta.ellpack_cols_;
    auto ell_tiles_total = (tt_meta.cell_count + 31u) / 32u;
    auto data_format = tt::DataFormat::Float32;
    uint32_t ell_tile_page_size = tt_metal::detail::TileSize(data_format);
    int vector_page_size = 1024;
    auto vec_entries_per_chunk = vector_page_size / 4u;
    auto vec_tile_h_per_chunk = vec_entries_per_chunk / 32u;
    auto vec_chunks_total = (ell_tiles_total + vec_tile_h_per_chunk -1) / vec_tile_h_per_chunk; // 1024 byte pages -> 256 floats -> 8 tiles for each vec-chunk

    uint32_t batch_size = diag_wb? 4u : 8u;

    auto batches_total = (ell_tiles_total + batch_size - 1) / batch_size;

    auto avail_cores = device->compute_with_storage_grid_size();

    auto [num_cores, used_cores, core_group_1, core_group_2, work_per_core1, work_per_core2] =
        tt::tt_metal::split_work_to_cores(avail_cores, batches_total);

    #ifdef ENABLE_DAISY_RTL
        if (region_id != 0) {
            __daisy_instrumentation_increment(region_id, "tt_used_cores", num_cores);
        }
    #endif

    #if TT_DEBUG > 0
    std::cout << "Using " << num_cores << " cores to process " << batches_total << " batches, " << batch_size << " tiles each ("
              << work_per_core1 << " on " << core_group_1.num_cores() << ", " << work_per_core2 << " on " << core_group_2.num_cores() << "; " << vec_chunks_total << " vec chunks (" << vec_entries_per_chunk << " floats/chunk))" << std::endl;
    #endif

    auto input_tile_count = batch_size * 2;
    auto vector_chunk_count = 32u; // at least
    auto result_page_count = 4;

    size_t vector_size = vector_page_size * 2;

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

    auto res_buf_page_size = (diag_wb? tt_metal::detail::TileSize(data_format) : vector_page_size);
    tt_metal::CreateCircularBuffer(
        program,
        used_cores,  // create on all cores
        tt_metal::CircularBufferConfig(
            res_buf_page_size * result_page_count,
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
            vector_page_size * vector_chunk_count,
            {
                {CBIndex::c_3, data_format}
            }
        )
        .set_page_size(CBIndex::c_3, vector_page_size)
    );

    std::vector<uint32_t> rd_compile_args, rd_common_args;
    tt_metal::TensorAccessorArgs(tt_meta.d_ellpack_vals_).append_to(rd_compile_args, rd_common_args);
    tt_metal::TensorAccessorArgs(tt_meta.d_ellpack_addrs_).append_to(rd_compile_args, rd_common_args);
    tt_metal::TensorAccessorArgs(d_inVec).append_to(rd_compile_args, rd_common_args);
    auto kernel_rd_0 = tt_metal::CreateKernel(
        program,
        kernel_dir / "ellpack" / (0? "mat_vec_reader_collection.cpp" : "mat_vec_reader_naive.cpp"),
        used_cores,
        tt_metal::ReaderDataMovementConfig(
            rd_compile_args
        )
    );

    std::vector<uint32_t> wr_compile_args, wr_common_args;
    tt_metal::TensorAccessorArgs(d_resVec).append_to(wr_compile_args, wr_common_args);
    auto kernel_wr_0 = tt_metal::CreateKernel(
        program,
        kernel_dir / "ellpack" / (diag_wb? "vec_diag_result_wb.cpp" : "vec_bare_result_wb.cpp"),
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
            vec_chunks_total,
            1, // vec_chunk_batch_size
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
            vec_chunks_total,
            1, // vec_chunk_batch_size
            1 // stream vec
        }
    );

    wr_common_args.insert(
        wr_common_args.begin(),
        {
            d_resVec.address(),
            batch_size,
        }
    );

    tt_metal::SetCommonRuntimeArgs(
        program,
        kernel_wr_0,
        wr_common_args
    );


    uint32_t start_batch = 0;
    uint32_t end_tile = ell_tiles_total; // ex

    for (auto& range : used_cores.ranges()) {

        for (auto& core : range) {
            uint32_t units;
            if (core_group_1.contains(core)) {
                units = work_per_core1;
            } else if (core_group_2.contains(core)) {
                units = work_per_core2;
            } else {
                units = 0;
            }

            auto tiles = units * batch_size;
            auto start_tile = start_batch * batch_size;
            if (start_tile + tiles > end_tile) {
                tiles = end_tile - start_tile;
            }

            tt::tt_metal::SetRuntimeArgs(
                program,
                kernel_rd_0,
                core,
                {
                    start_tile,
                    tiles,
                    units,
                    batch_size
                }
            );

            tt::tt_metal::SetRuntimeArgs(
                program,
                kernel_comp_0,
                core,
                {
                    units,
                    batch_size,
                    tiles
                }
            );

            tt::tt_metal::SetRuntimeArgs(
                program,
                kernel_wr_0,
                core,
                {
                    start_batch,
                    units,
                    start_tile,
                    tiles,
                }
            );

            start_batch += units;
        }
    }

    tt_metal::EnqueueProgram(device->command_queue(0), program, false);
}

}   // namespace tt::daisy::foam
