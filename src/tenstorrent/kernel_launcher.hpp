
#pragma once

#undef OPEN_MPI
#define OMPI_HAS_ULFM 0
#include <tt-metalium/host_api.hpp>


class KernelLauncher;

class KernelLauncher {
private:
    tt::tt_metal::IDevice* device_;
    

public:
    KernelLauncher():
        device_(tt::tt_metal::CreateDevice(0))
    {
        
    }

    ~KernelLauncher() {
        if (device_) {
            tt::tt_metal::CloseDevice(device_);
        }
    }
};

static KernelLauncher* kernelLauncher = nullptr;

KernelLauncher& require_kernel_launcher();
