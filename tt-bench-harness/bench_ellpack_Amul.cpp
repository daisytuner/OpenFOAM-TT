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
#include "ellpack_matVec.hpp"
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

using namespace tt::daisy;
using namespace tt::daisy::foam;

int main() {

    auto kernel_dir = std::string(std::getenv("TT_FOAM_KERNEL_DIR"));
    tt::tt_metal::IDevice* device = tt::tt_metal::CreateDevice(0);

    auto Nx = 8;
    auto Ny = 8;
    Foam::label cells = Nx * Ny;


    auto triang_size = 4*cells;
    Foam::labelList addr_upper(triang_size);
    Foam::labelList addr_lower(triang_size);

    // Walk the upper triangle of a square matrix (excluding diagonal)
    int idx = 0;
    // OpenFOAM cell numbering: i + j*Nx
    for (Foam::label j = 0; j < Ny; ++j) {
        for (Foam::label i = 0; i < Nx; ++i) {
            Foam::label addr = i + j * Nx;

            if (i < Nx - 1) {
                addr_lower[idx] = addr;
                addr_upper[idx] = addr + 1;
                ++idx;
            }
            // South neighbor (j+1)
            if (j < Ny - 1) {
                addr_lower[idx] = addr;
                addr_upper[idx] = addr + Nx;
                ++idx;
            }
        }
    }

    bool is_nnz = false;
    for (int i = 0; i < cells; ++i) {
        for (int j = 0; j < cells; ++j) {
            is_nnz = false;
            if (i == j) { std::cout << i << "," << j << " "; continue; }
            for (int k = 0; k < idx; ++k) {
                if ((i == addr_lower[k] && j == addr_upper[k])) {
                    std::cout << addr_lower[k] << "," << addr_upper[k] << " ";
                    is_nnz = true;
                    break;
                }
                else if ((j == addr_lower[k] && i == addr_upper[k])) {
                    std::cout << addr_upper[k] << "," << addr_lower[k] << " ";
                    is_nnz = true;
                    break;
                }
            }
            if (!is_nnz) {
                std::cout << 0 << " ";
            }
        }
        std::cout << std::endl;
    }

    // Resize arrays to actual number of off-diagonal entries
    addr_lower.setSize(idx);
    addr_upper.setSize(idx);

    Foam::lduPrimitiveMesh mesh(
            cells,
            addr_lower,
            addr_upper,
            0, // comm
            true
    );

    Foam::lduMatrix lduA(mesh);
    lduA.diag() = 3.0;
//    lduA.lower() = 0.0; // no values at all means mirrored from upper, 1.0 as well
    lduA.upper() = 1.0;

    Foam::scalarField inVec(cells, 2.0);
    for (int i = 0; i < cells; ++i) {
        inVec[i] = static_cast<float>(i+1);
    }
    Foam::scalarField result(cells);

    Foam::Info << "lduA: " << lduA << Foam::endl;
//    Foam::Info << "Input: " << inVec << Foam::endl;

    auto tt_meta_a = get_tt_meta(&lduA, ldu_tt_meta_map);
    auto tile_size = tt::tt_metal::detail::TileSize(tt::DataFormat::Float32);
    auto cells_aligned = tt::round_up(cells, tt::constants::TILE_WIDTH);

    BufferPool buffer_pool(device);

    // copying starts

    tt::daisy::foam::copy_ldu_to_ellpack(buffer_pool, tt_meta_a, &lduA);

    tt::tt_metal::Finish(device->command_queue(0));

    auto& d_inVec = tt::daisy::foam::copy_scalarField_to_device_bare(buffer_pool, inVec);

    tt::tt_metal::Finish(device->command_queue(0));

    auto& d_resVec = buffer_pool.allocateBuffer(d_inVec.buffer->size(), d_inVec.buffer->page_size());;

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

    tt_launch_ellpack_matVecOp(
        device,
        tt_meta_a,
        *d_inVec.buffer,
        *d_resVec.buffer,
        kernel_dir
    );

    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata = {
        .file_name = "bench_ellpack_Amul.cpp",
        .function_name = "main",
        .line_begin = 29,
        .line_end = 192,
        .column_begin = 0,
        .column_end = 0,
        .target_type = "TENSTORRENT",
        .region_uuid = "foam_ellpack_Amul"
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_NONE);
    __daisy_instrumentation_enter(region_id);
    #endif


    tt_launch_ellpack_matVecOp(
        device,
        tt_meta_a,
        *d_inVec.buffer,
        *d_resVec.buffer,
        kernel_dir
    );

    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_exit(region_id);
        uint32_t num_tiles = (lduA.diag().size() + tt::constants::TILE_WIDTH - 1) / tt::constants::TILE_WIDTH;
        uint32_t vec_tiles_total = (lduA.diag().size() + 31) / 32;
        uint32_t batch_tiles = 8;
        uint32_t ell_tile_page_size = 4096;
        uint32_t vec_page_size = 1024;
        uint32_t nnz = lduA.diag().size() + lduA.lower().size() + lduA.upper().size();
        uint32_t reads =  num_tiles * 2 * ell_tile_page_size
                         + vec_tiles_total * vec_page_size;
        uint32_t writes = vec_tiles_total * vec_page_size;
        uint32_t flops = 2 * nnz;
        __daisy_instrumentation_increment(region_id, "flop", flops);
        __daisy_instrumentation_increment(region_id, "dram_bytes", reads + writes);
        __daisy_instrumentation_finalize(region_id);
    #endif

    tt::tt_metal::Finish(device->command_queue(0));

    tt::daisy::foam::copy_scalarField_from_device_bare(buffer_pool, d_resVec, &result);

    tt::tt_metal::Finish(device->command_queue(0));

    Foam::Info << "Result: " << result << Foam::endl;

    Foam::scalarField expected(cells, 6.0);

    if (!Foam::daisy::matches(result, expected)) {
        Foam::SeriousError << "FAIL Expected: " << expected << Foam::endl;
    }

    tt::tt_metal::CloseDevice(device);

    return 0;
}