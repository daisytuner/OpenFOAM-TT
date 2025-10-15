#include "ellpack_matVec.hpp"

#include <tt-metalium/host_api.hpp>
#include <tt-metalium/work_split.hpp>
#include <filesystem>
#include <tt-metalium/kernel_types.hpp>
#include <tt-metalium/tensor_accessor_args.hpp>

namespace tt::daisy {

void tt_launch_ellpack_matVecOp(
    tt::tt_metal::IDevice* device,
    tt::daisy::tt_ldu_meta& tt_meta,
    tt::tt_metal::Buffer& d_inVec,
    tt::tt_metal::Buffer& d_resVec,
    const std::filesystem::path& kernel_dir
) {

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
    uint32_t batch_size = 8u;

    auto batches_total = (ell_tiles_total + batch_size - 1) / batch_size;


    auto avail_cores = device->compute_with_storage_grid_size();

    auto [num_cores, used_cores, core_group_1, core_group_2, work_per_core1, work_per_core2] =
        tt::tt_metal::split_work_to_cores(avail_cores, batches_total);

    std::cout << "Using " << num_cores << " cores to process " << batches_total << " batches, " << batch_size << " tiles each ("
              << work_per_core1 << " on " << core_group_1.num_cores() << ", " << work_per_core2 << " on " << core_group_2.num_cores() << "; " << vec_chunks_total << " vec chunks (" << vec_entries_per_chunk << " floats/chunk))" << std::endl;

    auto input_tile_count = batch_size * 2;
    auto vector_chunk_count = 32u; // at least

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
                {CBIndex::c_2, tt::DataFormat::UInt32}
            }
        )
        .set_page_size(CBIndex::c_2, ell_tile_page_size));

    tt_metal::CreateCircularBuffer(
        program,
        used_cores,  // create on all cores
        tt_metal::CircularBufferConfig(
            vector_page_size * vector_chunk_count,
            {
                {CBIndex::c_0, data_format},
            }
        )
        .set_page_size(CBIndex::c_0, vector_page_size)
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
        kernel_dir / "ellpack" / "mat_vec_reader_naive.cpp",
        used_cores,
        tt_metal::ReaderDataMovementConfig(
            rd_compile_args
        )
    );

    std::vector<uint32_t> wr_compile_args, wr_common_args;
    tt_metal::TensorAccessorArgs(d_resVec).append_to(wr_compile_args, wr_common_args);
    auto kernel_wr_0 = tt_metal::CreateKernel(
        program,
        kernel_dir / "ellpack" / "vec_bare_result_wb.cpp",
        used_cores,
        tt_metal::WriterDataMovementConfig(
            wr_compile_args
        )
    );

    auto kernel_comp_0 = tt_metal::CreateKernel(
        program,
        kernel_dir / "ellpack" / "mat_vec_compute_naive.cpp",
        used_cores,
        tt_metal::ComputeConfig {
            .math_fidelity = MathFidelity::HiFi4,
            .fp32_dest_acc_en = true,
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
            vec_chunks_total
        }
    );

    wr_common_args.insert(
        wr_common_args.begin(),
        {
            d_resVec.address(),
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
                    units // units is in 1 vec-page so the minimum the WB can do
                }
            );

            start_batch += units;
        }
    }

    tt_metal::EnqueueProgram(device->command_queue(0), program, false);
}

}   // namespace tt::daisy::foam
