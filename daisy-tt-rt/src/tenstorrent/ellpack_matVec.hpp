#pragma once

#include "ttLduData.hpp"
#include <cstdint>
#include <tt-metalium/host_api.hpp>

namespace tt::daisy {

void tt_launch_ellpack_matVecOp(
    tt::tt_metal::IDevice* device,
    tt_ldu_meta& tt_meta,
    tt::tt_metal::Buffer& d_inVec,
    tt::tt_metal::Buffer& d_resVec,
    uint32_t cells, // rows and columns in the unpacked matrix and lines in the vector
    uint32_t packed_cols, // columns in the packed matrix
    std::filesystem::path kernel_dir = std::filesystem::current_path()
);

}   // namespace tt::daisy::foam
