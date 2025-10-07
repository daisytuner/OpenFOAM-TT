#pragma once

#include "tt-metalium/device.hpp"
#include <tt-metalium/host_api.hpp>

namespace tt::daisy {

struct TTDenseMatOpAssignKernelMeta {
    tt::tt_metal::Program program;
    tt::tt_metal::IDevice* device;
    std::variant<CoreCoord, CoreRange, CoreRangeSet> cores;
    size_t page_size;

    tt::tt_metal::KernelHandle kernel_rd_0;
    tt::tt_metal::KernelHandle kernel_wr_0;
    tt::tt_metal::KernelHandle kernel_comp_0;
};

void tt_setup_dense_matOpAssign_program(
    TTDenseMatOpAssignKernelMeta& program,
    tt::tt_metal::IDevice* device,
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

void tt_launch_dense_matOpAssign(
    tt::tt_metal::IDevice* device,
    tt::tt_metal::Buffer& d_a,
    tt::tt_metal::Buffer& d_b,
    tt::tt_metal::Buffer& d_res,
    uint32_t mat_width, // assumes square matrices
    std::string opSymbol,
    std::filesystem::path kernel_dir
);

}  // namespace tt::daisy