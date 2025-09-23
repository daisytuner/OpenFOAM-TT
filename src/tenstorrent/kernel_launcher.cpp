#include "kernel_launcher.hpp"
#include "ReusableTtBuffer.hpp"
#include "tt-metalium/buffer.hpp"
#include "ttLduData.hpp"
#include <cassert>
#include <tt-metalium/host_api.hpp>

static KernelLauncher* kernelLauncher = nullptr;

KernelLauncher& require_kernel_launcher() {
    if (!kernelLauncher) {
        kernelLauncher = new KernelLauncher();
    }
    return *kernelLauncher;
}

KernelLauncher::KernelLauncher():
        device_(tt::tt_metal::CreateDevice(0))
{
    init_amul_program();
    init_suma_program();
    init_residual_program();
}

KernelLauncher::~KernelLauncher() {
    for (auto buffer : buffers_) {
        delete buffer;
    }
    if (device_) {
        tt::tt_metal::CloseDevice(device_);
    }
}


ReusableTtBuffer& KernelLauncher::allocateBuffer(size_t size) {
    bool found = false;
    auto it = buffers_.begin();
    ReusableTtBuffer* cur = nullptr;
    while (!found && it != buffers_.end()) {
        cur = *it;
        if (cur->free && cur->buffer->size() >= size) {
            found = true;
            cur->free = false;
            break;
        }
        ++it;
    }

    if (!found) {
        auto buf = new ReusableTtBuffer(tt::round_up(size, 1024), device_);
        buffers_.push_back(buf);
        return *buf;
    } else {
        return *cur;
    }
}

void KernelLauncher::freeBuffer(ReusableTtBuffer& buffer) {
    buffer.free = true;
}

void KernelLauncher::init_amul_program() {
    auto& p = program_amul_;
    tt::tt_metal::CoreCoord all_cores = device_->compute_with_storage_grid_size();
    tt::tt_metal::CoreCoord one_core = {0, 0};

    auto addr_cb_config = tt::tt_metal::CircularBufferConfig(program_amul_.addr_size_alloc, {{0, tt::DataFormat::UInt32}})
        .set_page_size(0, 4096);
    auto addr_cb = tt::tt_metal::CreateCircularBuffer(program_amul_.program, one_core, addr_cb_config);
    auto data_cb_config = tt::tt_metal::CircularBufferConfig(program_amul_.data_size_alloc, {{1, tt::DataFormat::UInt32}})
        .set_page_size(1, 4096);
    auto data_cb = tt::tt_metal::CreateCircularBuffer(program_amul_.program, one_core, data_cb_config);
    auto psi_cb_config = tt::tt_metal::CircularBufferConfig(program_amul_.psi_size_alloc, {{2, tt::DataFormat::UInt32}})
        .set_page_size(2, 4096);
    auto psi_cb = tt::tt_metal::CreateCircularBuffer(program_amul_.program, one_core, psi_cb_config);
    auto Apsi_cb_config = tt::tt_metal::CircularBufferConfig(program_amul_.Apsi_size_alloc, {{3, tt::DataFormat::UInt32}})
        .set_page_size(3, 4096);
    auto Apsi_cb = tt::tt_metal::CreateCircularBuffer(program_amul_.program, one_core, Apsi_cb_config);
    auto iface_cb_config = tt::tt_metal::CircularBufferConfig(program_amul_.iface_size_alloc, {{4, tt::DataFormat::UInt32}})
        .set_page_size(4, 4096);
    auto iface_cb = tt::tt_metal::CreateCircularBuffer(program_amul_.program, one_core, iface_cb_config);

    auto kernel_naive = tt::tt_metal::CreateKernel(program_amul_.program, "/home/ramon/git/OpenFOAM-TT/src/tenstorrent-kernels/ldu_Amul_dataCore.cpp", one_core, tt::tt_metal::ReaderDataMovementConfig({ .compile_args = {} }));
    program_amul_.kernel_0 = kernel_naive;

}

void KernelLauncher::launch_amul(
    const tt_ldu_meta& lduMeta,
    tt::tt_metal::Buffer& d_psi,
    tt::tt_metal::Buffer& d_Apsi,
    // tt::tt_metal::Buffer& d_iface_contents,
    // int iface_count,
    const Foam::direction cmpt
) {

    assert(d_psi.size() <= static_cast<uint32_t>(program_amul_.psi_size_alloc));
    assert(lduMeta.d_data_->size() <= static_cast<uint32_t>(program_amul_.data_size_alloc));
    assert(lduMeta.d_addrs_->size() <= static_cast<uint32_t>(program_amul_.addr_size_alloc));
    assert(d_Apsi.size() <= static_cast<uint32_t>(program_amul_.Apsi_size_alloc));
    
    tt::tt_metal::SetRuntimeArgs(
        program_amul_.program,
        program_amul_.kernel_0,
        tt::tt_metal::CoreCoord {0, 0},
        {
            lduMeta.d_addrs_->address(),
            lduMeta.upper_addrs_start_,
            lduMeta.iface_map_start_,
            lduMeta.d_data_->address(),
            lduMeta.cell_count,
            lduMeta.lower_contents_start_,
            lduMeta.sparse_count,
            lduMeta.upper_contents_start_,
            d_psi.address(),
            d_Apsi.address(),
            // d_iface_contents.address(),
            // static_cast<uint32_t>(iface_count),
        }
    );

    
    tt::tt_metal::EnqueueProgram(device_->command_queue(0), program_amul_.program, false);

}

void KernelLauncher::init_suma_program() {
    auto& p = program_suma_;
    tt::tt_metal::CoreCoord all_cores = device_->compute_with_storage_grid_size();
    tt::tt_metal::CoreCoord one_core = {1, 1};

    auto addr_cb_config = tt::tt_metal::CircularBufferConfig(program_suma_.addr_size_alloc, {{0, tt::DataFormat::UInt32}})
        .set_page_size(0, 4096);
    auto addr_cb = tt::tt_metal::CreateCircularBuffer(program_suma_.program, one_core, addr_cb_config);
    auto data_cb_config = tt::tt_metal::CircularBufferConfig(program_suma_.data_size_alloc, {{1, tt::DataFormat::UInt32}})
        .set_page_size(1, 4096);
    auto data_cb = tt::tt_metal::CreateCircularBuffer(program_suma_.program, one_core, data_cb_config);
    auto res_cb_config = tt::tt_metal::CircularBufferConfig(program_suma_.res_size_alloc, {{2, tt::DataFormat::UInt32}})
        .set_page_size(2, 4096);
    auto res_cb = tt::tt_metal::CreateCircularBuffer(program_suma_.program, one_core, res_cb_config);
    auto iface_cb_config = tt::tt_metal::CircularBufferConfig(program_suma_.iface_size_alloc, {{3, tt::DataFormat::UInt32}})
        .set_page_size(3, 4096);
    auto iface_cb = tt::tt_metal::CreateCircularBuffer(program_suma_.program, one_core, iface_cb_config);

    auto kernel_naive = tt::tt_metal::CreateKernel(program_suma_.program, "/home/ramon/git/OpenFOAM-TT/src/tenstorrent-kernels/ldu_sumA_dataCore.cpp", one_core, tt::tt_metal::ReaderDataMovementConfig({ .compile_args = {} }));
    program_suma_.kernel_0 = kernel_naive;
}

void KernelLauncher::launch_suma(
    const tt_ldu_meta& lduMeta,
    tt::tt_metal::Buffer& d_res,
    tt::tt_metal::Buffer& d_iface_contents,
    int iface_count
) {

    assert(d_res.size() <= static_cast<uint32_t>(program_suma_.res_size_alloc));
    assert(lduMeta.d_data_->size() <= static_cast<uint32_t>(program_suma_.data_size_alloc));
    assert(lduMeta.d_addrs_->size() <= static_cast<uint32_t>(program_suma_.addr_size_alloc));
    
    tt::tt_metal::SetRuntimeArgs(
        program_suma_.program,
        program_suma_.kernel_0,
        tt::tt_metal::CoreCoord {1, 1},
        {
            lduMeta.d_addrs_->address(),
            lduMeta.upper_addrs_start_,
            lduMeta.iface_map_start_,
            lduMeta.d_data_->address(),
            lduMeta.cell_count,
            lduMeta.lower_contents_start_,
            lduMeta.sparse_count,
            lduMeta.upper_contents_start_,
            d_res.address(),
            d_iface_contents.address(),
            static_cast<uint32_t>(iface_count),
        }
    );

    
    tt::tt_metal::EnqueueProgram(device_->command_queue(0), program_suma_.program, false);

}

void KernelLauncher::init_residual_program() {
    auto& p = program_residual_;
    tt::tt_metal::CoreCoord all_cores = device_->compute_with_storage_grid_size();
    tt::tt_metal::CoreCoord one_core = {2, 2};

    auto addr_cb_config = tt::tt_metal::CircularBufferConfig(program_residual_.addr_size_alloc, {{0, tt::DataFormat::UInt32}})
        .set_page_size(0, 4096);
    auto addr_cb = tt::tt_metal::CreateCircularBuffer(program_residual_.program, one_core, addr_cb_config);
    auto data_cb_config = tt::tt_metal::CircularBufferConfig(program_residual_.data_size_alloc, {{1, tt::DataFormat::UInt32}})
        .set_page_size(1, 4096);
    auto data_cb = tt::tt_metal::CreateCircularBuffer(program_residual_.program, one_core, data_cb_config);
    auto psi_cb_config = tt::tt_metal::CircularBufferConfig(program_residual_.psi_size_alloc, {{2, tt::DataFormat::UInt32}})
        .set_page_size(2, 4096);
    auto psi_cb = tt::tt_metal::CreateCircularBuffer(program_residual_.program, one_core, psi_cb_config);
    auto source_cb_config = tt::tt_metal::CircularBufferConfig(program_residual_.source_size_alloc, {{3, tt::DataFormat::UInt32}})
        .set_page_size(3, 4096);
    auto source_cb = tt::tt_metal::CreateCircularBuffer(program_residual_.program, one_core, source_cb_config);
    auto res_cb_config = tt::tt_metal::CircularBufferConfig(program_residual_.res_size_alloc, {{4, tt::DataFormat::UInt32}})
        .set_page_size(4, 4096);
    auto res_cb = tt::tt_metal::CreateCircularBuffer(program_residual_.program, one_core, res_cb_config);


    auto kernel_naive = tt::tt_metal::CreateKernel(program_residual_.program, "/home/ramon/git/OpenFOAM-TT/src/tenstorrent-kernels/ldu_residual_dataCore.cpp", one_core, tt::tt_metal::ReaderDataMovementConfig({ .compile_args = {} }));
    program_residual_.kernel_0 = kernel_naive;
}

void KernelLauncher::launch_residual(
    const tt_ldu_meta& lduMeta,
    tt::tt_metal::Buffer& d_psi,
    tt::tt_metal::Buffer& d_source,
    tt::tt_metal::Buffer& d_res,
    // tt::tt_metal::Buffer& d_iface_contents,
    // int iface_count,
    const Foam::direction cmpt
) {

    assert(d_res.size() <= static_cast<uint32_t>(program_residual_.res_size_alloc));
    assert(lduMeta.d_data_->size() <= static_cast<uint32_t>(program_residual_.data_size_alloc));
    assert(lduMeta.d_addrs_->size() <= static_cast<uint32_t>(program_residual_.addr_size_alloc));
    assert(d_psi.size() <= static_cast<uint32_t>(program_residual_.psi_size_alloc));
    assert(d_source.size() <= static_cast<uint32_t>(program_residual_.source_size_alloc));
    
    tt::tt_metal::SetRuntimeArgs(
        program_residual_.program,
        program_residual_.kernel_0,
        tt::tt_metal::CoreCoord {2, 2},
        {
            lduMeta.d_addrs_->address(),
            lduMeta.upper_addrs_start_,
            lduMeta.iface_map_start_,
            lduMeta.d_data_->address(),
            lduMeta.cell_count,
            lduMeta.lower_contents_start_,
            lduMeta.sparse_count,
            lduMeta.upper_contents_start_,
            d_psi.address(),
            d_source.address(),
            d_res.address(),
            // d_iface_contents.address(),
            // static_cast<uint32_t>(iface_count),
        }
    );

    
    tt::tt_metal::EnqueueProgram(device_->command_queue(0), program_residual_.program, false);

}
