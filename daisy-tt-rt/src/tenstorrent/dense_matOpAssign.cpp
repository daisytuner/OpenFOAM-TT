#include "dense_matOpAssign.hpp"
#include "tt-metalium/kernel_types.hpp"


void tt_setup_dense_matOpAssign_program(
    TTDenseMatOpAssignKernelMeta& program,
    tt::tt_metal::CoreCoord& cores,
    std::string opSymbol,
    std::filesystem::path kernel_dir
) {
    int page_size = 4096;
    int buf_size = page_size * 2;

    auto used_cores = cores;

    auto dest_data_cb_config = tt::tt_metal::CircularBufferConfig(buf_size, {{0, tt::DataFormat::Float32}})
            .set_page_size(0, page_size);

    auto dest_data_cb = tt::tt_metal::CreateCircularBuffer(program.program, used_cores, dest_data_cb_config);

    auto a_data_cb_config = tt::tt_metal::CircularBufferConfig(buf_size, {{1, tt::DataFormat::Float32}})
        .set_page_size(1, page_size);

    auto a_data_cb = tt::tt_metal::CreateCircularBuffer(program.program, used_cores, a_data_cb_config);

    auto b_data_cb_config = tt::tt_metal::CircularBufferConfig(buf_size, {{2, tt::DataFormat::Float32}})
        .set_page_size(2, page_size);

    auto b_data_cb = tt::tt_metal::CreateCircularBuffer(program.program, used_cores, b_data_cb_config);

    program.kernel_rd_0 = tt::tt_metal::CreateKernel(
        program.program,
        (kernel_dir / "dense" / "matOpAssign_rd.cpp").string(),
        used_cores,
        tt::tt_metal::ReaderDataMovementConfig({}, {{"KERNEL_OP", opSymbol}})
    );

    program.kernel_rd_0 = tt::tt_metal::CreateKernel(
        program.program,
        (kernel_dir / "ldu" / "ldu_matOpAssign_dataCore.cpp").string(),
        used_cores,
        tt::tt_metal::ComputeConfig {
            .math_fidelity = MathFidelity::HiFi4,
            .compile_args = {},
            .defines = {{"KERNEL_OP", opSymbol}}
        }
    );

    program.kernel_rd_0 = tt::tt_metal::CreateKernel(
        program.program,
        (kernel_dir / "ldu" / "matOpAssign_wr.cpp").string(),
        used_cores,
        tt::tt_metal::WriterDataMovementConfig({}, {{"KERNEL_OP", opSymbol}})
    );
}

void tt_launch_dense_matOpAssign(
    TTDenseMatOpAssignKernelMeta& program,
    tt::tt_metal::Buffer& d_a,
    tt::tt_metal::Buffer& d_b,
    tt::tt_metal::Buffer& d_dest,
    uint32_t mat_width
) {
    throw std::runtime_error("tt_launch_dense_matOpAssign not implemented yet");
}