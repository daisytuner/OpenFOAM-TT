#pragma once

#include <tt-metalium/host_api.hpp>
#include <cstdint>

namespace tt::daisy {

/**
 * C (MxN) = A (MxK) * B (KxN)
 */
void tt_launch_dense_matMul(
    tt_metal::IDevice* device,
    tt_metal::Buffer& d_a,
    tt_metal::Buffer& d_b,
    tt_metal::Buffer& d_output,
    uint32_t M,
    uint32_t N,
    uint32_t K,
    uint32_t B = 1,
    bool bcast_batch = false,
    std::filesystem::path kernel_dir = std::filesystem::current_path()
);

void tt_launch_dense_matMul_large(
    tt_metal::IDevice* device,
    tt_metal::Buffer& d_a,
    tt_metal::Buffer& d_b,
    tt_metal::Buffer& d_output,
    uint32_t M,
    uint32_t N,
    uint32_t K,
    uint32_t B = 1,
    bool bcast_batch = false,
    std::filesystem::path kernel_dir = std::filesystem::current_path()
);

void tt_launch_dense_matMul_small(
    tt_metal::IDevice* device,
    tt_metal::Buffer& d_a,
    tt_metal::Buffer& d_b,
    tt_metal::Buffer& d_output,
    uint32_t M,
    uint32_t N,
    uint32_t K,
    uint32_t B = 1,
    bool bcast_batch = false,
    std::filesystem::path kernel_dir = std::filesystem::current_path()
);

}   // namespace tt::daisy