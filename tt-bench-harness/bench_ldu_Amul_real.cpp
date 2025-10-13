
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

using namespace tt::daisy;
using namespace tt::daisy::foam;

int main() {

    tt::tt_metal::IDevice* device = tt::tt_metal::CreateDevice(0);

    BufferPool buffer_pool(device);

    auto kernel_dir = std::string(std::getenv("TT_FOAM_KERNEL_DIR"));

    Foam::label cells = 4096;


tt::tt_metal::IDevice* device = tt::tt_metal::CreateDevice(0);
BufferPool buffer_pool(device);

auto kernel_dir = std::string(std::getenv("TT_FOAM_KERNEL_DIR"));

const int Nx = 400;
const int Ny = 400;
const Foam::label cells = Nx * Ny;

// Maximum number of off-diagonal entries:
// interior cells have 4 neighbors, boundary cells have 2 or 3
// allocate maximum possible: 4 * cells
Foam::labelList addr_upper(4 * cells);
Foam::labelList addr_lower(4 * cells);

int idx = 0;

// OpenFOAM cell numbering: i + j*Nx
for (Foam::label j = 0; j < Ny; ++j) {
    for (Foam::label i = 0; i < Nx; ++i) {
        Foam::label row = i + j * Nx;

        // West neighbor (i-1)
        if (i > 0) {
            addr_lower[idx] = row;
            addr_upper[idx] = row - 1;
            ++idx;
        }
        // East neighbor (i+1)
        if (i < Nx - 1) {
            addr_lower[idx] = row;
            addr_upper[idx] = row + 1;
            ++idx;
        }
        // South neighbor (j-1)
        if (j > 0) {
            addr_lower[idx] = row;
            addr_upper[idx] = row - Nx;
            ++idx;
        }
        // North neighbor (j+1)
        if (j < Ny - 1) {
            addr_lower[idx] = row;
            addr_upper[idx] = row + Nx;
            ++idx;
        }
    }
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

// Fill values
lduA.diag() = 4.0;  // interior cells have 4 neighbors
lduA.lower() = -1.0;
lduA.upper() = -1.0;

// Optionally, adjust diagonal for boundary cells
for (Foam::label j = 0; j < Ny; ++j) {
    for (Foam::label i = 0; i < Nx; ++i) {
        Foam::label row = i + j * Nx;
        Foam::scalar diag = 0.0;
        if (i > 0) diag += 1.0;
        if (i < Nx - 1) diag += 1.0;
        if (j > 0) diag += 1.0;
        if (j < Ny - 1) diag += 1.0;
        lduA.diag()[row] = diag;
    }
}


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
        __daisy_instrumentation_increment(region_id, "flop", 4 * tt_meta.sparse_count);
        uint32_t page_size = tile_size
        uint32_t M = aligned_cells
        uint32_t N = 32
        uint32_t K = aligned_cells
        uint32_t Kt = K / TILE_WIDTH
        uint32_t num_output_tiles = (M * N) / TILE_HW
        uint32_t num_tiles = num_output_tiles;

        uint32_t reads = num_output_tiles * Kt  * (2 * page_size) * sizeof(float)
        uint32_t writes = num_tiles * page_size * sizeof(float)

        uint32_t flops = num_output_tiles * 2 * page_size * page_size
        __daisy_instrumentation_increment(region_id, "dram_bytes", reads + writes);
        __daisy_instrumentation_finalize(region_id);
    #endif

    tt::daisy::foam::copy_scalarField_from_device_dense_mat(buffer_pool, d_resVec, &result);

    tt::tt_metal::Finish(device->command_queue(0));

    Foam::Info << "Result: " << result << Foam::endl;

    Foam::scalarField expected(cells, 6.0);

    if (!Foam::daisy::matches(result, expected)) {
        Foam::SeriousError << "FAIL Expected: " << expected << Foam::endl;
    }

    tt::tt_metal::CloseDevice(device);

    return 0;
}