#pragma once

#include <cstdint>
#include <tt-metalium/host_api.hpp>

namespace tt::daisy {

enum class EllpackHwImpl {
    None = 0,
    FPU = 1,
    SFPU = 2
};

constexpr EllpackHwImpl default_ellpack_hw_impl = EllpackHwImpl::None;

void tt_launch_ellpack_matVecOp(
    tt::tt_metal::IDevice* device,
    uint32_t cells,
    uint32_t ellpack_cols,
    tt::tt_metal::Buffer& d_ellpack_vals,
    tt::tt_metal::Buffer& d_ellpack_addrs,
    tt::tt_metal::Buffer& d_inVec,
    tt::tt_metal::Buffer& d_resVec,
    uint32_t* ellpack_first_col_per_tile,
    uint32_t* ellpack_last_col_per_tile,
    const std::filesystem::path& kernel_dir = std::filesystem::current_path(),
    EllpackHwImpl hwImpl = default_ellpack_hw_impl,
    size_t region_id = 0 // 0 is invalid
);

}   // namespace tt::daisy
