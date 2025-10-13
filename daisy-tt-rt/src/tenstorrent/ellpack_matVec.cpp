#include "ellpack_matVec.hpp"

#include <tt-metalium/host_api.hpp>
#include <tt-metalium/work_split.hpp>
#include <filesystem>
#include <tt-metalium/kernel_types.hpp>
#include <tt-metalium/tensor_accessor_args.hpp>

namespace tt::daisy {

void tt_launch_ellpack_matVecOp(
    tt::tt_metal::IDevice* device,
    tt::daisy::tt_ldu_meta& tt_meta,
    tt::tt_metal::Buffer& d_inVec,
    tt::tt_metal::Buffer& d_resVec,
    uint32_t cells, // rows and columns in the unpacked matrix and lines in the vector
    uint32_t packed_cols, // columns in the packed matrix
    std::filesystem::path kernel_dir
) {
    
    throw std::runtime_error("ellpack matVec not implemented yet");

}

}   // namespace tt::daisy::foam
