#include "dense_matOpAssign.hpp"
#include "tt-metalium/core_coord.hpp"
#include <cstdint>
#include <tt-metalium/host_api.hpp>
#include <tt-metalium/kernel_types.hpp>
#include <tt-metalium/tensor_accessor_args.hpp>

namespace tt::daisy {

void tt_setup_dense_matOpAssign_program(
    TTDenseMatOpAssignKernelMeta& program,
    tt::tt_metal::IDevice* device,
    tt::tt_metal::Buffer& d_a,
    tt::tt_metal::Buffer& d_b,
    tt::tt_metal::Buffer& d_dest,
    std::string opSymbol,
    std::filesystem::path kernel_dir
) {
    int page_size = 4096;
    int buf_size = page_size * 2;

    auto used_cores = device->compute_with_storage_grid_size();
    program.device = device;
    program.cores = used_cores;

    auto dest_data_cb_config = tt::tt_metal::CircularBufferConfig(buf_size, {{0, tt::DataFormat::Float32}})
            .set_page_size(0, page_size);

    auto dest_data_cb = tt::tt_metal::CreateCircularBuffer(program.program, used_cores, dest_data_cb_config);

    auto a_data_cb_config = tt::tt_metal::CircularBufferConfig(buf_size, {{1, tt::DataFormat::Float32}})
        .set_page_size(1, page_size);

    auto a_data_cb = tt::tt_metal::CreateCircularBuffer(program.program, used_cores, a_data_cb_config);

    auto b_data_cb_config = tt::tt_metal::CircularBufferConfig(buf_size, {{2, tt::DataFormat::Float32}})
        .set_page_size(2, page_size);

    auto b_data_cb = tt::tt_metal::CreateCircularBuffer(program.program, used_cores, b_data_cb_config);

    std::vector<uint32_t> rd_compile_args;
    tt::tt_metal::TensorAccessorArgs(d_a).append_to(rd_compile_args); // which would need to be runtime, so that we can reuse the same compiled kernel with different buffers?
    tt::tt_metal::TensorAccessorArgs(d_b).append_to(rd_compile_args);

    program.kernel_rd_0 = tt::tt_metal::CreateKernel(
        program.program,
        (kernel_dir / "dense" / "matOpAssign_rd.cpp").string(),
        used_cores,
        tt::tt_metal::ReaderDataMovementConfig(rd_compile_args, {})
    );

    program.kernel_rd_0 = tt::tt_metal::CreateKernel(
        program.program,
        (kernel_dir / "dense" / "matOpAssign_fpu.cpp").string(),
        used_cores,
        tt::tt_metal::ComputeConfig {
            .math_fidelity = MathFidelity::HiFi4,
            .fp32_dest_acc_en = true,
            .math_approx_mode = false,
            .compile_args = {},
            .defines = {{"KERNEL_OP", opSymbol}}
        }
    );

    std::vector<uint32_t> wr_compile_args;
    tt::tt_metal::TensorAccessorArgs(d_dest).append_to(wr_compile_args);

    program.kernel_rd_0 = tt::tt_metal::CreateKernel(
        program.program,
        (kernel_dir / "dense" / "dense_matrix_wb.cpp").string(),
        used_cores,
        tt::tt_metal::WriterDataMovementConfig(wr_compile_args, {})
    );
}

void tt_launch_dense_matOpAssign(
    TTDenseMatOpAssignKernelMeta& program,
    tt::tt_metal::Buffer& d_a,
    tt::tt_metal::Buffer& d_b,
    tt::tt_metal::Buffer& d_dest,
    uint32_t mat_width
) {
    
    tt::tt_metal::SetRuntimeArgs(
        program.program,
        program.kernel_rd_0,
        tt::tt_metal::CoreCoord {1, 1},
        {
            d_a.address(),
            d_b.address(),
            d_dest.address(),
            mat_width,
            mat_width * mat_width, // total elements
            0, // offset a
            0, // offset b
            0, // offset dest
        }
    );

    tt::tt_metal::EnqueueProgram(program.device->command_queue(0), program.program, false);
}

void tt_launch_dense_matOpAssign(
    tt::tt_metal::IDevice* device,
    tt::tt_metal::Buffer& d_a,
    tt::tt_metal::Buffer& d_b,
    tt::tt_metal::Buffer& d_res,
    uint32_t mat_width, // assumes square matrices
    std::string opSymbol,
    std::filesystem::path kernel_dir
) {
    TTDenseMatOpAssignKernelMeta p;

    tt_setup_dense_matOpAssign_program(
        p,
        device,
        d_a,
        d_b,
        d_res,
        opSymbol,
        kernel_dir
    );

    tt_launch_dense_matOpAssign(
        p,
        d_a,
        d_b,
        d_res,
        mat_width
    );

}

}