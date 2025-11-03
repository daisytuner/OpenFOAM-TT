#pragma once

#include "tt-metalium/device.hpp"
#include <tt-metalium/host_api.hpp>

namespace tt::daisy {

enum class MatBinOp {
    ADD = 0,
    SUB = 1
};

void tt_launch_dense_matBinOp(
    tt::tt_metal::IDevice* device,
    tt::tt_metal::Buffer& d_a,
    tt::tt_metal::Buffer& d_b,
    tt::tt_metal::Buffer& d_res,
    uint32_t mat_width, // assumes square matrices
    MatBinOp opSymbol,
    std::filesystem::path kernel_dir = std::filesystem::current_path()
);

}  // namespace tt::daisy

