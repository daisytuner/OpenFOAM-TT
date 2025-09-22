#pragma once

#include "ReusableTtBuffer.hpp"
#include <tt-metalium/buffer.hpp>
#include <tt-metalium/host_api.hpp>
#include <vector>
#include <direction.H>
#include "ttLduData.hpp"



struct AmulKernelMeta {
    tt::tt_metal::Program program;
    int addr_size_alloc = 8192;
    int data_size_alloc = 8192;
    int iface_size_alloc = 8192;
    int psi_size_alloc = 8192;
    int Apsi_size_alloc = 8192;
    tt::tt_metal::KernelHandle kernel_0;
};

class KernelLauncher {
public:
    tt::tt_metal::IDevice* device_;

private:
    std::vector<ReusableTtBuffer*> buffers_;

    AmulKernelMeta program_amul_;

public:
    KernelLauncher():
        device_(tt::tt_metal::CreateDevice(0))
    {
        init_amul_program();
    }

    ~KernelLauncher() {
        for (auto buffer : buffers_) {
            delete buffer;
        }
        if (device_) {
            tt::tt_metal::CloseDevice(device_);
        }
    }

    ReusableTtBuffer& allocateBuffer(size_t size);
    void freeBuffer(ReusableTtBuffer& buffer);

    void launch_amul(
        const tt_ldu_meta& lduMeta,
        tt::tt_metal::Buffer& d_psi,
        tt::tt_metal::Buffer& d_Apsi,
        tt::tt_metal::Buffer& d_iface_contents,
        int iface_count,
        const Foam::direction cmpt
    );

    void init_amul_program();
};

KernelLauncher& require_kernel_launcher();
