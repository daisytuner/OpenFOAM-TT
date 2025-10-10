#pragma once

#include "tt-metalium/device.hpp"
#include <tt-metalium/host_api.hpp>

namespace tt::daisy {

void tt_launch_dense_matNeg(
    tt::tt_metal::IDevice* device,
    tt::tt_metal::Buffer& d_a,
    tt::tt_metal::Buffer& d_res,
    uint32_t mat_width, // assumes square matrices
    std::filesystem::path kernel_dir = std::filesystem::current_path()
);

}  // namespace tt::daisy

