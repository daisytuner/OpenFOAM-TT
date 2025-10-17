#include "dense_matNeg.hpp"
#include "tt-metalium/base_types.hpp"
#include "tt-metalium/circular_buffer_constants.h"
#include "tt-metalium/core_coord.hpp"
#include "tt-metalium/program.hpp"
#include <cstdint>
#include <tt-metalium/host_api.hpp>
#include <tt-metalium/work_split.hpp>
#include <tt-metalium/kernel_types.hpp>
#include <tt-metalium/tensor_accessor_args.hpp>
#include <vector>

namespace tt::daisy {

void tt_launch_dense_matNeg(
    tt::tt_metal::IDevice* device,
    tt::tt_metal::Buffer& d_a,
    tt::tt_metal::Buffer& d_dest,
    uint32_t mat_width,
    std::filesystem::path kernel_dir
) {
    tt::tt_metal::Program program;

    auto padded_dim = tt::round_up(mat_width, 32);
    auto tiles_dim = padded_dim / 32;
    auto num_output_tiles_total = tiles_dim * tiles_dim;

    auto avail_cores = device->compute_with_storage_grid_size();
    
    auto [num_cores, used_cores, core_group_1, core_group_2, work_per_core1, work_per_core2] =
        tt::tt_metal::split_work_to_cores(avail_cores, num_output_tiles_total);

    std::cout << "Using " << num_cores << " cores to process " << num_output_tiles_total << " tiles. ("
              << work_per_core1 << " on " << core_group_1.num_cores() << ", " << work_per_core2 << " on " << core_group_2.num_cores() << ")" << std::endl;

    int page_size = 4096;
    int buf_size = page_size * 2;

    auto dest_data_cb_config = tt::tt_metal::CircularBufferConfig(buf_size, {{0, tt::DataFormat::Float32}})
            .set_page_size(0, page_size);

    auto dest_data_cb = tt::tt_metal::CreateCircularBuffer(program, used_cores, dest_data_cb_config);

    auto a_data_cb_config = tt::tt_metal::CircularBufferConfig(buf_size, {{1, tt::DataFormat::Float32}})
        .set_page_size(1, page_size);

    auto a_data_cb = tt::tt_metal::CreateCircularBuffer(program, used_cores, a_data_cb_config);

    std::vector<uint32_t> rd_compile_args, rd_common_args;
    tt::tt_metal::TensorAccessorArgs(d_a).append_to(rd_compile_args, rd_common_args);

    auto kernel_rd_0 = tt::tt_metal::CreateKernel(
        program,
        (kernel_dir / "dense" / "mat1_rd.cpp").string(),
        used_cores,
        tt::tt_metal::ReaderDataMovementConfig(rd_compile_args, {})
    );

    auto kernel_comp_0 = tt::tt_metal::CreateKernel(
        program,
        (kernel_dir / "dense" / "matNeg_sfpu.cpp").string(),
        used_cores,
        tt::tt_metal::ComputeConfig {
            .math_fidelity = MathFidelity::HiFi4,
            .fp32_dest_acc_en = true,
            .unpack_to_dest_mode = std::vector<UnpackToDestMode>(NUM_CIRCULAR_BUFFERS, UnpackToDestMode::UnpackToDestFp32),
            .compile_args = {},
        }
    );

    std::vector<uint32_t> wr_compile_args, wr_common_args;
    tt::tt_metal::TensorAccessorArgs(d_dest).append_to(wr_compile_args, wr_common_args);

    auto kernel_wr_0 = tt::tt_metal::CreateKernel(
        program,
        (kernel_dir / "dense" / "dense_matrix_wb.cpp").string(),
        used_cores,
        tt::tt_metal::WriterDataMovementConfig(wr_compile_args, {})
    );

    auto a_addr = d_a.address();
    auto dest_addr = d_dest.address();

    uint32_t start_tile = 0;

    rd_common_args.insert(
        rd_common_args.begin(),
        {
            a_addr
        }
    );

    tt::tt_metal::SetCommonRuntimeArgs(
        program,
        kernel_rd_0,
        rd_common_args
    );

    wr_common_args.insert(
        wr_common_args.begin(),
        {
            dest_addr,
        }
    );

    tt::tt_metal::SetCommonRuntimeArgs(
        program,
        kernel_wr_0,
        wr_common_args
    );

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

            tt::tt_metal::SetRuntimeArgs(
                program,
                kernel_rd_0,
                core,
                {
                    start_tile,
                    units
                }
            );

            tt::tt_metal::SetRuntimeArgs(
                program,
                kernel_comp_0,
                core,
                {
                    units
                }
            );

            tt::tt_metal::SetRuntimeArgs(
                program,
                kernel_wr_0,
                core,
                {
                    start_tile,
                    units
                }
            );

            start_tile += units;
        }
    }

    tt::tt_metal::EnqueueProgram(device->command_queue(0), program, false);
}

}