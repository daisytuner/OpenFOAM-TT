#pragma once

#include <tt-metalium/host_api.hpp>

struct TTDenseMatOpAssignKernelMeta {
    tt::tt_metal::Program program;
    
    size_t page_size;

    tt::tt_metal::KernelHandle kernel_rd_0;
    tt::tt_metal::KernelHandle kernel_wr_0;
    tt::tt_metal::KernelHandle kernel_comp_0;
};

void tt_setup_dense_matOpAssign_program(
    TTDenseMatOpAssignKernelMeta& program,
    tt::tt_metal::CoreCoord& cores,
    std::string opSymbol,
    std::filesystem::path kernel_dir = std::filesystem::current_path()
);

void tt_launch_dense_matOpAssign(
    TTDenseMatOpAssignKernelMeta& program,
    tt::tt_metal::Buffer& d_a,
    tt::tt_metal::Buffer& d_b,
    tt::tt_metal::Buffer& d_dest,
    uint32_t mat_width
);