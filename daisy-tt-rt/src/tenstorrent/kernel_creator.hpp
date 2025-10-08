#pragma once

#include <tt-metalium/host_api.hpp>

namespace tt::daisy {

tt_metal::KernelHandle CreateKernel(
    tt_metal::Program& program,
    std::optional<std::filesystem::path> kernel_dir,
    const std::filesystem::path& kfile,
    const std::string& kstring,
    const std::variant<tt_metal::CoreCoord, tt_metal::CoreRange, tt_metal::CoreRangeSet>& core_spec,
    const std::variant<tt_metal::DataMovementConfig, tt_metal::ComputeConfig, tt_metal::EthernetConfig>& config
);

#define TT_KERNEL_IDENTIFIER(p1, p2) tt_kernel_##p1##_##p2
#define TT_KERNEL_PATH(p1, p2) (std::filesystem::path("##p1##) / std::filesystem::path(p2))

#define TT_KERNEL(p1, p2) TT_KERNEL_PATH(p1, p2), TT_KERNEL_IDENTIFIER(p1, p2)

}  // namespace tt::daisy