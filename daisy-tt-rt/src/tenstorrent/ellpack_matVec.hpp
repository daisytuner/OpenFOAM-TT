#pragma once

#include "ttLduData.hpp"
#include <cstdint>
#include <tt-metalium/host_api.hpp>

namespace tt::daisy {

enum class EllpackHwImpl {
    None = 0,
    FPU = 1,
    SFPU = 2
};

void tt_launch_ellpack_matVecOp(
    tt::tt_metal::IDevice* device,
    tt::daisy::tt_ldu_meta& tt_meta,
    tt::tt_metal::Buffer& d_inVec,
    tt::tt_metal::Buffer& d_resVec,
    const std::filesystem::path& kernel_dir = std::filesystem::current_path(),
    EllpackHwImpl hwImpl = EllpackHwImpl::None
);

std::tuple<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t> calculate_ellpack_matVec_metrics(
    const tt::daisy::tt_ldu_meta& tt_meta,
    EllpackHwImpl hwImpl = EllpackHwImpl::None
);

}   // namespace tt::daisy::foam
