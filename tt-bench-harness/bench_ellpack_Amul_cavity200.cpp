#ifdef ENABLE_DAISY_RTL
#include <daisy_rtl/daisy_rtl.h>
#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <tt-metalium/host_api.hpp>
#include <unistd.h>

#include "dense_matBinOp.hpp"
#include "dense_matMul.hpp"
#include "device_transfers.hpp"
#include "ellpack_matVec.hpp"
#include "error.H"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"
#include "ldu_meta_cache.hpp"
#include "messageStream.H"
#include "result_matchers.hpp"
#include "scalarField.H"
#include "tt-metalium/buffer.hpp"
#include "tt-metalium/profiler_types.hpp"
#include "tt-metalium/tt_metal_profiler.hpp"
#include "ttLduData.hpp"
#include "Field.H"
#include "tmp.H"
#include "ref_Amul.hpp"

#include "cavity-mesh.hpp"

#ifndef ELLPACK_HW_IMPL
#define ELLPACK_HW_IMPL EllpackHwImpl::None
#endif

using namespace tt::daisy;
using namespace tt::daisy::foam;

int main() {

    auto kernel_dir = std::string(std::getenv("TT_FOAM_KERNEL_DIR"));
    tt::tt_metal::IDevice* device = tt::tt_metal::CreateDevice(0);

    Foam::label cells = 200;

    Foam::labelList addr_upper(Foam::UList<Foam::label>(cavity_mesh_200_upper_addr ,sizeof(cavity_mesh_200_upper_addr)/4));
    Foam::labelList addr_lower(Foam::UList<Foam::label>(cavity_mesh_200_lower_addr ,sizeof(cavity_mesh_200_lower_addr)/4));

    Foam::lduPrimitiveMesh mesh(
            cells,
            addr_lower,
            addr_upper,
            0, // comm
            true
    );

    Foam::Info << "Mesh created with " << mesh.lduAddr().size() << " cells, " << addr_upper.size() << " address entries" << Foam::endl;

    Foam::lduMatrix lduA(mesh);

    lduA.diag() = Foam::scalarField(Foam::UList<Foam::scalar>(cavity_mat_200_diag ,sizeof(cavity_mat_200_diag)/4));
    lduA.upper() = Foam::scalarField(Foam::UList<Foam::scalar>(cavity_mat_200_upper ,sizeof(cavity_mat_200_upper)/4));

    Foam::scalarField inVec(Foam::UList<Foam::scalar>(cavity_inVec_200, sizeof(cavity_inVec_200)/4));
    Foam::scalarField result(cells);

    Foam::Info << "lduA: " << lduA << Foam::endl;
//    Foam::Info << "Input: " << inVec << Foam::endl;

    auto tt_meta_a = get_tt_meta(&lduA, ldu_tt_meta_map);
    auto cells_aligned = tt::round_up(cells, tt::constants::TILE_WIDTH);

    BufferPool buffer_pool(device);

    EllpackHwImpl impl = ELLPACK_HW_IMPL;
    std::cout << "Selected ellpack kernel impl: " << static_cast<int>(impl) << std::endl;

    // copying starts


    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata1 = {
        .file_name = "bench_ellpack_Amul.cpp",
        .function_name = "foam_ldu_h2d_ellpack",
        .line_begin = 150,
        .line_end = 153,
        .column_begin = 0,
        .column_end = 0,
        .element_type = "h2d_transfer",
        .target_type = "TENSTORRENT",
        .region_uuid = "foam_ldu_h2d_ellpack"
    };
    unsigned long long region_id1 = __daisy_instrumentation_init(&metadata1, __DAISY_EVENT_SET_NONE);
    __daisy_instrumentation_enter(region_id1);
    #endif

    tt::daisy::foam::copy_ldu_to_ellpack(buffer_pool, tt_meta_a, &lduA);

    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
        auto [dram_bytes_rd, dram_bytes_wr, mul_flops, add_flops, mat_h2d_bytes, vec_transfer_bytes] = calculate_ellpack_matVec_metrics(tt_meta_a, impl);    
        __daisy_instrumentation_exit(region_id1);
        __daisy_instrumentation_increment(region_id1, "pcie_bytes", 2*mat_h2d_bytes); // dat + addrs/mesh
        __daisy_instrumentation_finalize(region_id1);
    #endif

    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata2 = {
        .file_name = "bench_ellpack_Amul.cpp",
        .function_name = "foam_scalarField_h2d_bare",
        .line_begin = 173,
        .line_end = 180,
        .column_begin = 0,
        .column_end = 0,
        .element_type = "h2d_transfer",
        .target_type = "TENSTORRENT",
        .region_uuid = "foam_scalarField_h2d_bare"
    };
    unsigned long long region_id2 = __daisy_instrumentation_init(&metadata2, __DAISY_EVENT_SET_NONE);
    __daisy_instrumentation_enter(region_id2);
    #endif
    auto& d_inVec = tt::daisy::foam::copy_scalarField_to_device_bare(buffer_pool, inVec);

    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_exit(region_id2);
        __daisy_instrumentation_increment(region_id2, "pcie_bytes", vec_transfer_bytes);
        __daisy_instrumentation_finalize(region_id2);
    #endif


    auto& d_resWarmup = buffer_pool.allocateBuffer(d_inVec.buffer->size(), d_inVec.buffer->page_size());

    auto& d_resVec = buffer_pool.allocateBuffer(d_inVec.buffer->size(), d_inVec.buffer->page_size());

//    Foam::lduMatrix lduRes(mesh);
//
//    tt::daisy::foam::copy_ldu_from_ellpack(device, tt_meta_a, &lduRes.diag(), &lduRes.lower(), &lduRes.upper(), lduRes.lduAddr());
//
//    bool fail = false;
//    if (!Foam::daisy::matches(lduA.diag(), lduRes.diag())) {
//        Foam::SeriousError << "TT diag do not match!" << Foam::endl;
//        Foam::Info << "org  Result: " << lduA.diag() << Foam::endl;
//        Foam::Info << "new Result: " << lduRes.diag() << Foam::endl;
//        fail = true;
//    }
//
//    if (!Foam::daisy::matches(lduA.lower(), lduRes.lower())) {
//        Foam::SeriousError << "TT lower do not match!" << Foam::endl;
//        Foam::Info << "org  Result: " << lduA.lower() << Foam::endl;
//        Foam::Info << "new Result: " << lduRes.lower() << Foam::endl;
//        fail = true;
//    }
//
//    if (!Foam::daisy::matches(lduA.upper(), lduRes.upper())) {
//        Foam::SeriousError << "TT upper do not match!" << Foam::endl;
//        Foam::Info << "org  Result: " << lduA.upper() << Foam::endl;
//        Foam::Info << "new Result: " << lduRes.upper() << Foam::endl;
//        fail = true;
//    }
//
//    if (fail) {
//        throw new std::runtime_error("TT copy_ldu_to_dense / copy_ldu_from_dense results do not match!");
//    }
    
    // WARMUP

    tt_launch_ellpack_matVecOp(
        device,
        tt_meta_a,
        *d_inVec.buffer,
        *d_resWarmup.buffer,
        kernel_dir,
        impl
    );

    tt::tt_metal::Finish(device->command_queue(0));

    tt::tt_metal::detail::ReadDeviceProfilerResults(device);

    // Actual Measurement of Kernel

    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata3 = {
        .file_name = "bench_ellpack_Amul.cpp",
        .function_name = "foam_ellpack_Amul_kernel",
        .line_begin = 193,
        .line_end = 218,
        .column_begin = 0,
        .column_end = 0,
        .target_type = "TENSTORRENT",
        .region_uuid = "foam_ellpack_Amul_kernel"
    };
    unsigned long long region_id3 = __daisy_instrumentation_init(&metadata3, __DAISY_EVENT_SET_NONE);
    __daisy_instrumentation_enter(region_id3);
    #else
    unsigned long long region_id3 = 0;
    #endif

    tt_launch_ellpack_matVecOp(
        device,
        tt_meta_a,
        *d_inVec.buffer,
        *d_resVec.buffer,
        kernel_dir,
        impl,
        region_id3
    );

    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_exit(region_id3);
        __daisy_instrumentation_increment(region_id3, "flop", mul_flops + add_flops);
        __daisy_instrumentation_increment(region_id3, "dram_bytes", dram_bytes_rd + dram_bytes_wr);
        __daisy_instrumentation_finalize(region_id3);
    #endif

    tt::tt_metal::detail::ReadDeviceProfilerResults(device);

    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata4 = {
        .file_name = "bench_ellpack_Amul.cpp",
        .function_name = "foam_scalarField_d2h_bare",
        .line_begin = 295,
        .line_end = 296,
        .column_begin = 0,
        .column_end = 0,
        .element_type = "d2h_transfer",
        .target_type = "TENSTORRENT",
        .region_uuid = "foam_scalarField_d2h_bare"
    };
    unsigned long long region_id4 = __daisy_instrumentation_init(&metadata4, __DAISY_EVENT_SET_NONE);
    __daisy_instrumentation_enter(region_id4);
    #endif

    tt::daisy::foam::copy_scalarField_from_device_bare(buffer_pool, d_resVec, &result);

    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_exit(region_id4);
        __daisy_instrumentation_increment(region_id4, "pcie_bytes", vec_transfer_bytes);
        __daisy_instrumentation_finalize(region_id4);
    #endif


    Foam::scalarField expected(cells);

    refAmul(lduA, expected, inVec);

    // Foam::Info << "Result: " << result << Foam::endl;

    auto atol = impl == EllpackHwImpl::FPU ? Foam::daisy::DEFAULT_TF32_MATCHER_ATOL : Foam::daisy::DEFAULT_SP_MATCHER_ATOL;
    auto rtol = impl == EllpackHwImpl::FPU ? Foam::daisy::DEFAULT_TF32_MATCHER_RTOL : Foam::daisy::DEFAULT_SP_MATCHER_RTOL;

    if (!Foam::daisy::matches(result, expected, rtol, atol)) {
        Foam::SeriousError << "FAIL Expected: " << expected << Foam::endl;
        Foam::Info << "Result: " << result << Foam::endl;
    } else {
        Foam::Info << "PASS" << Foam::endl;
    }

    tt::tt_metal::CloseDevice(device);

    return 0;
}
