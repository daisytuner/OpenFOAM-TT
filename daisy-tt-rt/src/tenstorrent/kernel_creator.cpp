#include "kernel_creator.hpp"

#include <tt-metalium/host_api.hpp>
#include <tt-metalium/kernel_types.hpp>
#include <tt-metalium/program.hpp>

namespace tt::daisy {

tt_metal::KernelHandle CreateKernel(
    tt_metal::Program& program,
    std::optional<std::filesystem::path> kernel_dir,
    const std::filesystem::path& kfile,
    const std::string& kstring,
    const std::variant<tt_metal::CoreCoord, tt_metal::CoreRange, tt_metal::CoreRangeSet>& core_spec,
    const std::variant<tt_metal::DataMovementConfig, tt_metal::ComputeConfig, tt_metal::EthernetConfig>& config
) {
    if (kernel_dir->empty()) {
        return tt::tt_metal::CreateKernelFromString(
            program,
            kstring,
            core_spec,
            config
        );
    } else {
        return tt::tt_metal::CreateKernel(
            program,
            (kernel_dir.value() / kfile).string(),
            core_spec,
            config
        );
    }
}

}  // namespace tt::daisy