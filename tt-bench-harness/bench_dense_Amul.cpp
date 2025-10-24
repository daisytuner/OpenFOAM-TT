#ifdef ENABLE_DAISY_RTL
#include <daisy_rtl/daisy_rtl.h>
#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <tt-metalium/host_api.hpp>

#include "dense_matBinOp.hpp"
#include "dense_matMul.hpp"
#include "device_transfers.hpp"
#include "error.H"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"
#include "ldu_meta_cache.hpp"
#include "messageStream.H"
#include "result_matchers.hpp"
#include "scalarField.H"
#include "tt-metalium/buffer.hpp"
#include "ttLduData.hpp"
#include "Field.H"
#include "tmp.H"
#include "ref_Amul.hpp"

using namespace tt::daisy;
using namespace tt::daisy::foam;

int main() {

    tt::tt_metal::IDevice* device = tt::tt_metal::CreateDevice(0);

    BufferPool buffer_pool(device);

    auto kernel_dir = std::string(std::getenv("TT_FOAM_KERNEL_DIR"));

    Foam::label cells = 4096;


    auto triang_size = cells*(cells-1)/2;
    Foam::labelList addr_upper(triang_size);
    Foam::labelList addr_lower(triang_size);

    // Walk the upper triangle of a square matrix (excluding diagonal)
    int idx = 0;
    for (Foam::label row = 0; row < cells; ++row) {
        for (Foam::label col = row + 1; col < cells; ++col) {
            addr_upper[idx] = col;
            addr_lower[idx] = row;

            ++idx;
        }
    }

    Foam::lduPrimitiveMesh mesh(
            cells,
            addr_lower,
            addr_upper,
            0, // comm
            true
    );

    Foam::lduMatrix lduA(mesh);
    lduA.diag() = 3.0;
    lduA.lower() = 0.0;
    lduA.upper() = 0.0;

    Foam::scalarField inVec(cells, 2.0);
    Foam::scalarField result(cells);

    auto tt_meta_a = get_tt_meta(&lduA, ldu_tt_meta_map);
    auto tile_size = tt::tt_metal::detail::TileSize(tt::DataFormat::Float32);
    auto cells_aligned = tt::round_up(cells, tt::constants::TILE_WIDTH);


    // copying starts

    tt::daisy::foam::copy_ldu_to_dense(device, tt_meta_a, &lduA);

    tt::tt_metal::Finish(device->command_queue(0));

    auto& d_inVec = tt::daisy::foam::copy_scalarField_to_device_as_dense_mat(buffer_pool, inVec);

    tt::tt_metal::Finish(device->command_queue(0));

    auto& d_resVec = buffer_pool.allocateBuffer(d_inVec.buffer->size(), tile_size);

    // tt::daisy::foam::copy_ldu_from_dense(device, tt_meta_a, &lduRes.diag(), &lduRes.lower(), &lduRes.upper(), lduRes.lduAddr());

    // bool fail = false;
    // if (!Foam::daisy::matches(lduA.diag(), lduRes.diag())) {
    //     Foam::SeriousError << "TT diag do not match!" << Foam::endl;
    //     Foam::Info << "org  Result: " << lduA.diag() << Foam::endl;
    //     Foam::Info << "new Result: " << lduRes.diag() << Foam::endl;
    //     fail = true;
    // }

    // if (!Foam::daisy::matches(lduA.lower(), lduRes.lower())) {
    //     Foam::SeriousError << "TT lower do not match!" << Foam::endl;
    //     Foam::Info << "org  Result: " << lduA.lower() << Foam::endl;
    //     Foam::Info << "new Result: " << lduRes.lower() << Foam::endl;
    //     fail = true;
    // }

    // if (!Foam::daisy::matches(lduA.upper(), lduRes.upper())) {
    //     Foam::SeriousError << "TT upper do not match!" << Foam::endl;
    //     Foam::Info << "org  Result: " << lduA.upper() << Foam::endl;
    //     Foam::Info << "new Result: " << lduRes.upper() << Foam::endl;
    //     fail = true;
    // }

    // if (fail) {
    //     throw new std::runtime_error("TT copy_ldu_to_dense / copy_ldu_from_dense results do not match!");
    // }

    tt_launch_dense_matMul(
        device,
        *tt_meta_a.d_dense_,
        *d_inVec.buffer,
        *d_resVec.buffer,
        cells_aligned,
        32,
        cells_aligned,
        1,
        false,
        kernel_dir
    );

    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata = {
        .file_name = "bench_ldu_Amul.cpp",
        .function_name = "main",
        .line_begin = 25,
        .line_end = 182,
        .column_begin = 0,
        .column_end = 0,
        .target_type = "TENSTORRENT",
        .region_uuid = "foam_lduMatrix_Amul"
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_NONE);
    __daisy_instrumentation_enter(region_id);
    #endif

    tt_launch_dense_matMul(
        device,
        *tt_meta_a.d_dense_,
        *d_inVec.buffer,
        *d_resVec.buffer,
        cells_aligned,
        32,
        cells_aligned,
        1,
        false,
        kernel_dir
    );

    tt::tt_metal::Finish(device->command_queue(0));


    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_exit(region_id);
        uint32_t M = cells_aligned;
        uint32_t N = 32;
        uint32_t K = cells_aligned;
        uint32_t Kt = K / tt::constants::TILE_WIDTH;
        uint32_t num_output_tiles = (M * N) / tt::constants::TILE_HW;
        uint32_t num_tiles = num_output_tiles;

        uint32_t reads = num_output_tiles * Kt  * (2 * tt::constants::TILE_HW) * sizeof(float);
        uint32_t writes = num_tiles * tt::constants::TILE_HW;

        uint32_t flops = num_output_tiles * Kt * 2 * tt::constants::TILE_HW * tt::constants::TILE_WIDTH;
        __daisy_instrumentation_increment(region_id, "flop", flops); // ~ 2 * M * N * K
        __daisy_instrumentation_increment(region_id, "dram_bytes", reads + writes);
        __daisy_instrumentation_finalize(region_id);
    #endif

    tt::daisy::foam::copy_scalarField_from_device_dense_mat(buffer_pool, d_resVec, &result);

    tt::tt_metal::Finish(device->command_queue(0));

    Foam::scalarField expected(cells);

    refAmul(lduA, expected, inVec);

    if (!Foam::daisy::matches(result, expected, Foam::daisy::DEFAULT_TF32_MATCHER_RTOL, Foam::daisy::DEFAULT_TF32_MATCHER_ATOL)) {
        Foam::SeriousError << "FAIL Expected: " << expected << Foam::endl;
        Foam::Info << "Result: " << result << Foam::endl;
    }

    tt::tt_metal::CloseDevice(device);

    return 0;
}