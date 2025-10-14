#pragma once

#include "ttLduData.hpp"
#include <cstdint>
#include <tt-metalium/host_api.hpp>

namespace tt::daisy {

void tt_launch_ellpack_matVecOp(
    tt::tt_metal::IDevice* device,
    tt::daisy::tt_ldu_meta& tt_meta,
    tt::tt_metal::Buffer& d_inVec,
    tt::tt_metal::Buffer& d_resVec,
    const std::filesystem::path& kernel_dir
);

}   // namespace tt::daisy::foam
