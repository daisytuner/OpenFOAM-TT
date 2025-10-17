#pragma once

#include <tt-metalium/buffer.hpp>
#include <tt-metalium/host_api.hpp>
#include "ReusableTtBuffer.hpp"
#include <vector>
#include "buffer_pool.hpp"
#include "dense_matBinOp.hpp"
#include "ttLduData.hpp"

namespace tt::daisy::foam {

struct AmulKernelMeta {
    tt::tt_metal::Program program;
    int addr_size_alloc = 8192;
    int data_size_alloc = 8192;
    int iface_size_alloc = 8192;
    int psi_size_alloc = 8192;
    int Apsi_size_alloc = 8192;
    tt::tt_metal::KernelHandle kernel_0;
};

struct SumAKernelMeta {
    tt::tt_metal::Program program;
    int addr_size_alloc = 8192;
    int data_size_alloc = 8192;
    int iface_size_alloc = 8192;
    int res_size_alloc = 8192;
    tt::tt_metal::KernelHandle kernel_0;
};

struct ResidualKernelMeta {
    tt::tt_metal::Program program;
    int addr_size_alloc = 8192;
    int data_size_alloc = 8192;
    int psi_size_alloc = 8192;
    int source_size_alloc = 8192;
    int res_size_alloc = 8192;
    tt::tt_metal::KernelHandle kernel_0;
};

struct LduMatInplaceKernelMeta {
    tt::tt_metal::Program program;
    int addr_size_alloc = 8192;
    int data_size_alloc = 8192;
    tt::tt_metal::KernelHandle kernel_0;
};

struct LduMatOpAssignkernelMeta {
    tt::tt_metal::Program program;
    int dest_data_size_alloc = 8192;
    int a_data_size_alloc = 8192;
    tt::tt_metal::KernelHandle kernel_0;
};

class KernelLauncher : public BufferPool {
public:

    std::filesystem::path kernel_dir_;

private:

    AmulKernelMeta program_amul_;
    SumAKernelMeta program_suma_;
    ResidualKernelMeta program_residual_;
    LduMatInplaceKernelMeta program_posSumDiag_;
    LduMatInplaceKernelMeta program_negSumDiag_;
    LduMatInplaceKernelMeta program_negate_;
    LduMatOpAssignkernelMeta program_matAddAssign_;
    LduMatOpAssignkernelMeta program_matSubAssign_;

    void launch_sumDiag(
        const tt_ldu_meta& lduMeta,
        LduMatInplaceKernelMeta& program
    );

    void launch_matOpAssign(
        tt_ldu_meta& lduDestMeta,
        const tt_ldu_meta& lduAMeta,
        LduMatOpAssignkernelMeta& program
    );

    void init_amul_program();
    void init_suma_program();
    void init_residual_program();
    void init_sumDiag_program();
    void init_negate_program();
    void init_matOpAssign_program();

public:
    KernelLauncher();
    ~KernelLauncher();

    void launch_amul(
        const tt_ldu_meta& lduMeta,
        tt::tt_metal::Buffer& d_psi,
        tt::tt_metal::Buffer& d_Apsi,
        const char cmpt
    );

    void launch_amul_with_interfaces(
        const tt_ldu_meta& lduMeta,
        tt::tt_metal::Buffer& d_psi,
        tt::tt_metal::Buffer& d_Apsi,
        tt::tt_metal::Buffer& d_iface_contents,
        int iface_count
    );

    void launch_suma(
        const tt_ldu_meta& lduMeta,
        tt::tt_metal::Buffer& d_res
        // tt::tt_metal::Buffer* d_iface_contents,
        // int iface_count
    );

    void launch_residual(
        const tt_ldu_meta& lduMeta,
        tt::tt_metal::Buffer& d_psi,
        tt::tt_metal::Buffer& d_source,
        tt::tt_metal::Buffer& d_res
        // tt::tt_metal::Buffer& d_iface_contents,
        // int iface_count,
        // const char cmpt
    );

    void launch_sumDiag(
        const tt_ldu_meta& lduMeta
    );

    void launch_negSumDiag(
        const tt_ldu_meta& lduMeta
    );

    void launch_negate(
        const tt_ldu_meta& lduMeta
    );

    void launch_matAddAssign(
        tt_ldu_meta& lduDestMeta,
        const tt_ldu_meta& lduAMeta
    );

    void launch_matSubAssign(
        tt_ldu_meta& lduDestMeta,
        const tt_ldu_meta& lduAMeta
    );

    void launch_amul_decompressed(
        const tt_ldu_meta& lduMeta,
        tt::tt_metal::Buffer& d_psi,
        tt::tt_metal::Buffer& d_Apsi,
        const char cmpt
    );
};

KernelLauncher& require_kernel_launcher();

void tt_compute_amul(KernelLauncher& k, tt_ldu_meta& tt_meta, ReusableTtBuffer& tt_psi, ReusableTtBuffer& tt_Apsi);

void tt_compute_sumA(
    KernelLauncher& k,
    tt_ldu_meta& tt_meta,
    ReusableTtBuffer& tt_res
);

void tt_compute_residual(
    KernelLauncher& k,
    tt_ldu_meta& tt_meta,
    ReusableTtBuffer& tt_psi,
    ReusableTtBuffer& tt_source,
    ReusableTtBuffer& tt_res
);

void tt_compute_negate(
    KernelLauncher& k,
    tt_ldu_meta& tt_meta
);

void tt_compute_matBinOp(
    KernelLauncher& k,
    tt_ldu_meta& tt_meta,
    const tt_ldu_meta& a_tt_meta,
    const tt_ldu_meta& b_tt_meta,
    MatBinOp opSymbol
);


}   // namespace tt::daisy::foam