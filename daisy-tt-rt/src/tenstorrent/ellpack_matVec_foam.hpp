#pragma once

#include "ttLduData.hpp"
#include <cstdint>
#include <tt-metalium/host_api.hpp>
#include "ellpack_matVec.hpp"

namespace tt::daisy::foam {


void tt_launch_ellpack_matVecOp(
    tt::tt_metal::IDevice* device,
    tt::daisy::tt_ldu_meta& tt_meta,
    tt::tt_metal::Buffer& d_inVec,
    tt::tt_metal::Buffer& d_resVec,
    const std::filesystem::path& kernel_dir = std::filesystem::current_path(),
    EllpackHwImpl hwImpl = default_ellpack_hw_impl,
    size_t region_id = 0 // 0 is invalid
);

std::tuple<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t> calculate_ellpack_matVec_metrics(
    tt::tt_metal::IDevice* device,
    const tt::daisy::tt_ldu_meta& tt_meta,
    EllpackHwImpl hwImpl = default_ellpack_hw_impl
);

}   // namespace tt::daisy::foam
