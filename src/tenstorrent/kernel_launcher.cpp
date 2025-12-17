#include "kernel_launcher.hpp"
#include "ReusableTtBuffer.hpp"
#include <tt-metalium/buffer.hpp>
#include "buffer_pool.hpp"
#include "dense_matMul.hpp"
#include "dense_matBinOp.hpp"
#include "dense_matNeg.hpp"
#include "ellpack_matVec_foam.hpp"
#include "ttLduData.hpp"
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <tt-metalium/host_api.hpp>
#include "tt_impls.hpp"

namespace tt::daisy::foam {

static KernelLauncher* kernelLauncher = nullptr;

KernelLauncher& require_kernel_launcher() {
    if (!kernelLauncher) {
        kernelLauncher = new KernelLauncher();
    }
    return *kernelLauncher;
}

KernelLauncher::KernelLauncher():
        BufferPool(tt::tt_metal::CreateDevice(0))
{
    auto e = std::getenv("TT_FOAM_KERNEL_DIR");
    if (e) {
        kernel_dir_ = e;
    } else {
        kernel_dir_ = std::filesystem::current_path() / "daisy-tt-rt" / "src" / "tenstorrent-kernels";
    }
    std::cout << "expecting TT kernels in " << kernel_dir_ << std::endl;

    init_amul_program();
    init_suma_program();
    init_residual_program();
    init_sumDiag_program();
    init_negate_program();
    init_matOpAssign_program();
}

KernelLauncher::~KernelLauncher() {
    if (device_) {
        tt::tt_metal::CloseDevice(device_);
    }
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

    auto kernel_naive = tt::tt_metal::CreateKernel(
        program_amul_.program,
        (kernel_dir_ / "ldu" / "ldu_Amul_dataCore.cpp").string(),
        one_core,
        tt::tt_metal::ReaderDataMovementConfig()
    );
    program_amul_.kernel_0 = kernel_naive;

}

void KernelLauncher::launch_amul(
    const tt_ldu_meta& lduMeta,
    tt::tt_metal::Buffer& d_psi,
    tt::tt_metal::Buffer& d_Apsi,
    // tt::tt_metal::Buffer& d_iface_contents,
    // int iface_count,
    const char cmpt
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

void KernelLauncher::launch_amul_decompressed(
    const tt_ldu_meta& lduMeta,
    tt::tt_metal::Buffer& d_psi,
    tt::tt_metal::Buffer& d_Apsi,
    const char cmpt
) {

    // for (auto& core : device_->compute_with_storage_cores()) {

    //     tt::tt_metal::SetRuntimeArgs(
    //         program_amul_decompressed_,
    //         program_amul_decompressed_.kernel_rd_0,
    //         core,
    //         {
    //             lduMeta.d_dense_->address(),
    //             d_psi.address(),
    //             lduMeta.cell_count,
    //             d_psi.address(),
    //             d_Apsi.address(),
    //         }
    //     );

    //     tt::tt_metal::SetRuntimeArgs(
    //         program_amul_decompressed_,
    //         program_amul_decompressed_.kernel_wr_0,
    //         core,
    //         {
    //             lduMeta.d_dense_->address(),
    //             lduMeta.cell_count,
    //             d_psi.address(),
    //             d_Apsi.address(),
    //         }
    //     );

    //     tt::tt_metal::SetRuntimeArgs(
    //         program_amul_decompressed_,
    //         program_amul_decompressed_.kernel_comp_0,
    //         core,
    //         {
    //             workload_size_
    //         }
    //     );
    // }

    // tt::tt_metal::EnqueueProgram(device_->command_queue(0), program_amul_decompressed_.program, false);
}

void KernelLauncher::launch_amul_with_interfaces(
    const tt_ldu_meta& lduMeta,
    tt::tt_metal::Buffer& d_psi,
    tt::tt_metal::Buffer& d_Apsi,
    tt::tt_metal::Buffer& d_iface_contents,
    int iface_count
) {

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

    auto kernel_naive = tt::tt_metal::CreateKernel(
        program_suma_.program,
        (kernel_dir_ / "ldu" / "ldu_sumA_dataCore.cpp").string(),
        one_core,
        tt::tt_metal::ReaderDataMovementConfig()
    );
    program_suma_.kernel_0 = kernel_naive;
}

void KernelLauncher::launch_suma(
    const tt_ldu_meta& lduMeta,
    tt::tt_metal::Buffer& d_res
    // tt::tt_metal::Buffer* d_iface_contents,
    // int iface_count
) {

    assert(d_res.size() <= static_cast<uint32_t>(program_suma_.res_size_alloc));
    assert(lduMeta.d_data_->size() <= static_cast<uint32_t>(program_suma_.data_size_alloc));
    assert(lduMeta.d_addrs_->size() <= static_cast<uint32_t>(program_suma_.addr_size_alloc));
    // assert(!d_iface_contents || (d_iface_contents->size() <= static_cast<uint32_t>(program_suma_.iface_size_alloc)));
    
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
            // d_iface_contents? d_iface_contents->address() : 0,
            // static_cast<uint32_t>(iface_count),
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


    auto kernel_naive = tt::tt_metal::CreateKernel(
        program_residual_.program,
        (kernel_dir_ / "ldu" / "ldu_residual_dataCore.cpp").string(),
        one_core,
        tt::tt_metal::ReaderDataMovementConfig()
    );
    program_residual_.kernel_0 = kernel_naive;
}

void KernelLauncher::launch_residual(
    const tt_ldu_meta& lduMeta,
    tt::tt_metal::Buffer& d_psi,
    tt::tt_metal::Buffer& d_source,
    tt::tt_metal::Buffer& d_res
    // tt::tt_metal::Buffer& d_iface_contents,
    // int iface_count
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

void KernelLauncher::init_sumDiag_program() {

    tt::tt_metal::CoreCoord all_cores = device_->compute_with_storage_grid_size();
    tt::tt_metal::CoreCoord one_core = {3, 3};

    auto create_sumDiag_kernel = [&](LduMatInplaceKernelMeta& p, uint32_t neg_mode) {
        auto addr_cb_config = tt::tt_metal::CircularBufferConfig(p.addr_size_alloc, {{0, tt::DataFormat::UInt32}})
            .set_page_size(0, 4096);
        auto addr_cb = tt::tt_metal::CreateCircularBuffer(p.program, one_core, addr_cb_config);
        auto data_cb_config = tt::tt_metal::CircularBufferConfig(p.data_size_alloc, {{1, tt::DataFormat::UInt32}})
            .set_page_size(1, 4096);
        auto data_cb = tt::tt_metal::CreateCircularBuffer(p.program, one_core, data_cb_config);

        p.kernel_0 = tt::tt_metal::CreateKernel(
            p.program,
            (kernel_dir_ / "ldu" / "ldu_sumDiag_dataCore.cpp").string(),
            one_core,
            tt::tt_metal::ReaderDataMovementConfig({neg_mode})
        );
    };

    create_sumDiag_kernel(program_posSumDiag_, 0);
    create_sumDiag_kernel(program_negSumDiag_,1);
}

void KernelLauncher::launch_sumDiag(
    const tt_ldu_meta& lduMeta,
    LduMatInplaceKernelMeta& program
) {

    assert(lduMeta.d_data_->size() <= static_cast<uint32_t>(program.data_size_alloc));
    assert(lduMeta.d_addrs_->size() <= static_cast<uint32_t>(program.addr_size_alloc));
    
    tt::tt_metal::SetRuntimeArgs(
        program.program,
        program.kernel_0,
        tt::tt_metal::CoreCoord {3, 3},
        {
            lduMeta.d_addrs_->address(),
            lduMeta.upper_addrs_start_,
            lduMeta.iface_map_start_,
            lduMeta.d_data_->address(),
            lduMeta.cell_count,
            lduMeta.lower_contents_start_,
            lduMeta.sparse_count,
            lduMeta.upper_contents_start_,
        }
    );

    tt::tt_metal::EnqueueProgram(device_->command_queue(0), program.program, false);

}

void KernelLauncher::launch_sumDiag(
    const tt_ldu_meta& lduMeta
) {
    launch_sumDiag(lduMeta, program_posSumDiag_);
}

void KernelLauncher::launch_negSumDiag(
    const tt_ldu_meta& lduMeta
) {
    launch_sumDiag(lduMeta, program_negSumDiag_);
}

void KernelLauncher::init_negate_program() {

    tt::tt_metal::CoreCoord all_cores = device_->compute_with_storage_grid_size();
    tt::tt_metal::CoreCoord one_core = {2, 1};

    auto& p = program_negate_;
    auto addr_cb_config = tt::tt_metal::CircularBufferConfig(p.addr_size_alloc, {{0, tt::DataFormat::UInt32}})
        .set_page_size(0, 4096);
    auto addr_cb = tt::tt_metal::CreateCircularBuffer(p.program, one_core, addr_cb_config);
    auto data_cb_config = tt::tt_metal::CircularBufferConfig(p.data_size_alloc, {{1, tt::DataFormat::UInt32}})
        .set_page_size(1, 4096);
    auto data_cb = tt::tt_metal::CreateCircularBuffer(p.program, one_core, data_cb_config);

    p.kernel_0 = tt::tt_metal::CreateKernel(
        p.program,
        (kernel_dir_ / "ldu" / "ldu_negate_dataCore.cpp").string(),
        one_core,
        tt::tt_metal::ReaderDataMovementConfig()
    );
}

void KernelLauncher::launch_negate(
    const tt_ldu_meta& lduMeta
) {

    assert(lduMeta.d_data_->size() <= static_cast<uint32_t>(program_negate_.data_size_alloc));
    assert(lduMeta.d_addrs_->size() <= static_cast<uint32_t>(program_negate_.addr_size_alloc));
    
    tt::tt_metal::SetRuntimeArgs(
        program_negate_.program,
        program_negate_.kernel_0,
        tt::tt_metal::CoreCoord {2, 1},
        {
            lduMeta.d_data_->address(),
            lduMeta.diag_zero ? 0 : lduMeta.cell_count, // give fake 0 count to stop it from touching uninit data
            lduMeta.lower_contents_start_,
            lduMeta.sparse_count,
            lduMeta.upper_contents_start_,
        }
    );

    tt::tt_metal::EnqueueProgram(device_->command_queue(0), program_negate_.program, false);

}




void KernelLauncher::init_matOpAssign_program() {
    tt::tt_metal::CoreCoord all_cores = device_->compute_with_storage_grid_size();
    tt::tt_metal::CoreCoord one_core = {1, 2};


    auto create_opAssign_kernel = [&](LduMatOpAssignkernelMeta& p, std::string opDefine) {
    
        auto dest_data_cb_config = tt::tt_metal::CircularBufferConfig(p.dest_data_size_alloc, {{1, tt::DataFormat::UInt32}})
            .set_page_size(1, 4096);
        auto dest_data_cb = tt::tt_metal::CreateCircularBuffer(p.program, one_core, dest_data_cb_config);

        auto a_data_cb_config = tt::tt_metal::CircularBufferConfig(p.a_data_size_alloc, {{2, tt::DataFormat::UInt32}})
            .set_page_size(2, 4096);
        auto a_data_cb = tt::tt_metal::CreateCircularBuffer(p.program, one_core, a_data_cb_config);

        p.kernel_0 = tt::tt_metal::CreateKernel(
            p.program,
            (kernel_dir_ / "ldu" / "ldu_matOpAssign_dataCore.cpp").string(),
            one_core,
            tt::tt_metal::ReaderDataMovementConfig({}, {{"KERNEL_OP", opDefine}})
        );
    };

    create_opAssign_kernel(program_matAddAssign_, "+");
    create_opAssign_kernel(program_matSubAssign_, "-");
}

void KernelLauncher::launch_matOpAssign(
    tt_ldu_meta& lduDestMeta,
    const tt_ldu_meta& lduAMeta,
    LduMatOpAssignkernelMeta& program
) {
    assert(lduDestMeta.d_data_->size() <= static_cast<uint32_t>(program.dest_data_size_alloc));
    assert(lduAMeta.d_data_->size() <= static_cast<uint32_t>(program.a_data_size_alloc));

    uint32_t mode_mask = 0;
    if (lduDestMeta.lower_contains_also_upper) {
        mode_mask |= 0x1;
    }
    if (lduAMeta.lower_contains_also_upper) {
        mode_mask |= 0x2;
    }
    if (lduDestMeta.lower_contains_also_upper && !lduAMeta.lower_contains_also_upper) {
        if (lduDestMeta.lower_contents_start_ == lduDestMeta.upper_contents_start_) {
            throw std::runtime_error("Need to expand symmetric dest to asymmetric, but dest does not have the space for that reserved!!");
        }
        mode_mask |= 0x4;
    }
    if (lduAMeta.diag_zero) {
        mode_mask |= 0x8;
    }
    if (lduDestMeta.diag_zero) {
        mode_mask |= 0x10;
    }
    if (lduAMeta.triang_zero) {
        mode_mask |= 0x20;
    }
    if (lduDestMeta.triang_zero) {
        mode_mask |= 0x40;
    }
    
    tt::tt_metal::SetRuntimeArgs(
        program.program,
        program.kernel_0,
        tt::tt_metal::CoreCoord {1, 2},
        {
            lduDestMeta.d_data_->address(),
            lduDestMeta.cell_count,
            lduDestMeta.lower_contents_start_,
            lduDestMeta.sparse_count,
            lduDestMeta.upper_contents_start_,

            lduAMeta.d_data_->address(),
            lduAMeta.cell_count,
            lduAMeta.lower_contents_start_,
            lduAMeta.sparse_count,
            lduAMeta.upper_contents_start_,
            
            mode_mask
        }
    );

    tt::tt_metal::EnqueueProgram(device_->command_queue(0), program.program, false);


    if (lduDestMeta.lower_contains_also_upper && !lduAMeta.lower_contains_also_upper) {
        lduDestMeta.lower_contains_also_upper = false; // now the dest is no longer symmetric
    }
    if (lduDestMeta.diag_zero && !lduAMeta.diag_zero) {
        lduDestMeta.diag_zero = false; // now the dest diag is no longer zero
    }
    if (lduDestMeta.triang_zero && !lduAMeta.triang_zero) {
        lduDestMeta.triang_zero = false; // now the dest triang is no longer zero
    }
}

void KernelLauncher::launch_matAddAssign(
    tt_ldu_meta& lduDestMeta,
    const tt_ldu_meta& lduAMeta
) {
    launch_matOpAssign(lduDestMeta, lduAMeta, program_matAddAssign_);
}

void KernelLauncher::launch_matSubAssign(
    tt_ldu_meta& lduDestMeta,
    const tt_ldu_meta& lduAMeta
) {
    launch_matOpAssign(lduDestMeta, lduAMeta, program_matSubAssign_);
}

void tt_compute_amul(KernelLauncher& k, tt_ldu_meta& tt_meta, ReusableTtBuffer& tt_psi, ReusableTtBuffer& tt_Apsi, size_t region_id) {
    #if TT_IMPL == TT_IMPL_LDU
        k.launch_amul(tt_meta, *tt_psi.buffer, *tt_Apsi.buffer, 0);
    #elif TT_IMPL == TT_IMPL_DENSE
        auto aligned_cells = tt::round_up(tt_meta.cell_count, 32);
        tt_launch_dense_matMul(
            k.device_,
            *tt_meta.d_dense_,
            *tt_psi.buffer,
            *tt_Apsi.buffer,
            aligned_cells,
            32,
            aligned_cells,
            1,
            false,
            k.kernel_dir_
        );
    #elif TT_IMPL == TT_IMPL_ELLPACK
        tt_launch_ellpack_matVecOp(k.device_, tt_meta, *tt_psi.buffer, *tt_Apsi.buffer, k.kernel_dir_, default_ellpack_hw_impl, region_id);
    #else
        #error unknown TT IMPL TT_IMPL
    #endif
}

void tt_compute_sumA(
    KernelLauncher& k,
    tt_ldu_meta& tt_meta,
    ReusableTtBuffer& tt_res
) {
    #if TT_IMPL == TT_IMPL_LDU
        k.launch_suma(tt_meta, *tt_res.buffer);
    #elif TT_IMPL == TT_IMPL_DENSE
        throw std::runtime_error("suma not implemented for dense mat");
    #elif TT_IMPL == TT_IMPL_ELLPACK
        throw std::runtime_error("suma not implemented for ellpack mat");
    #else
        #error unknown TT IMPL TT_IMPL
    #endif
}

void tt_compute_residual(
    KernelLauncher& k,
    tt_ldu_meta& tt_meta,
    ReusableTtBuffer& tt_psi,
    ReusableTtBuffer& tt_source,
    ReusableTtBuffer& tt_res
) {
    #if TT_IMPL == TT_IMPL_LDU
        k.launch_residual(tt_meta, *tt_psi.buffer, *tt_source.buffer, *tt_res.buffer);
    #elif TT_IMPL == TT_IMPL_DENSE
        throw std::runtime_error("suma not implemented for dense mat");
    #elif TT_IMPL == TT_IMPL_ELLPACK
        throw std::runtime_error("suma not implemented for ellpack mat");
    #else
        #error unknown TT IMPL TT_IMPL
    #endif
}

void tt_compute_negate(
    KernelLauncher& k,
    tt_ldu_meta& tt_meta
) {
    #if TT_IMPL == TT_IMPL_LDU
        k.launch_negate(tt_meta);
    #elif TT_IMPL == TT_IMPL_DENSE
        tt_launch_dense_matNeg(
            k.device_,
            *tt_meta.d_dense_,
            *tt_meta.d_dense_,
            tt_meta.cell_count,
            k.kernel_dir_
        );
    #elif TT_IMPL == TT_IMPL_ELLPACK
        throw std::runtime_error("negate not implemented for ellpack mat");
    #else
        #error unknown TT IMPL TT_IMPL
    #endif
}

void tt_compute_matBinOp(
    KernelLauncher& k,
    tt_ldu_meta& tt_meta_res,
    const tt_ldu_meta& a_tt_meta,
    const tt_ldu_meta& b_tt_meta,
    MatBinOp opSymbol
) {
    #if TT_IMPL == TT_IMPL_LDU
        if (&tt_meta_res != &a_tt_meta) {
            throw std::runtime_error("for LDU matBinOp, the result mat must be the same as the first operand mat");
        }
        if (opSymbol == MatBinOp::ADD) {
            k.launch_matAddAssign(tt_meta_res, b_tt_meta);
        } else if (opSymbol == MatBinOp::SUB) {
            k.launch_matSubAssign(tt_meta_res, b_tt_meta);
        } else {
            throw std::runtime_error("unsupported opSymbol " + std::to_string(static_cast<int>(opSymbol)));
        }
    #elif TT_IMPL == TT_IMPL_DENSE
        tt_launch_dense_matBinOp(
            k.device_,
            *a_tt_meta.d_dense_,
            *b_tt_meta.d_dense_,
            *tt_meta_res.d_dense_,
            a_tt_meta.cell_count,
            opSymbol,
            k.kernel_dir_
        );
    #elif TT_IMPL == TT_IMPL_ELLPACK
        throw std::runtime_error("matBinOp not implemented for ellpack mat");
    #else
        #error unknown TT IMPL TT_IMPL
    #endif
}

}  // namespace tt::daisy::foam