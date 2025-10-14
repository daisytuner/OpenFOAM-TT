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

using namespace tt::daisy;
using namespace tt::daisy::foam;

int main(int argc, char* argv[]) {

    // Parse command line arguments
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <Nx> <Ny>" << std::endl;
        std::cerr << "  Nx: Number of cells in x direction" << std::endl;
        std::cerr << "  Ny: Number of cells in y direction" << std::endl;
        return 1;
    }

    int Nx, Ny;
    try {
        Nx = std::stoi(argv[1]);
        Ny = std::stoi(argv[2]);

        if (Nx <= 0 || Ny <= 0) {
            std::cerr << "Error: Nx and Ny must be positive integers" << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing command line arguments: " << e.what() << std::endl;
        std::cerr << "Usage: " << argv[0] << " <Nx> <Ny>" << std::endl;
        return 1;
    }

    std::cout << "Running with grid size: " << Nx << " x " << Ny << " = " << (Nx * Ny) << " cells" << std::endl;

    tt::tt_metal::IDevice* device = tt::tt_metal::CreateDevice(0);

    BufferPool buffer_pool(device);

    auto kernel_dir = std::string(std::getenv("TT_FOAM_KERNEL_DIR"));
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

    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata = {
        .file_name = "bench_ldu_Amul_real.cpp",
        .function_name = "main",
        .line_begin = 25,
        .line_end = 230,
        .column_begin = 0,
        .column_end = 0,
        .target_type = "TENSTORRENT",
        .region_uuid = "foam_lduMatrix_Amul_real"
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
        __daisy_instrumentation_increment(region_id, "flop", 4 * tt_meta_a.sparse_count);
        uint32_t page_size = tile_size;
        uint32_t M = cells_aligned;
        uint32_t N = 32;
        uint32_t K = cells_aligned;
        uint32_t Kt = K / tt::constants::TILE_WIDTH;
        uint32_t num_output_tiles = (M * N) / tt::constants::TILE_HW;
        uint32_t num_tiles = num_output_tiles;

        uint32_t reads = num_output_tiles * Kt  * (2 * page_size) * sizeof(float);
        uint32_t writes = num_tiles * page_size * sizeof(float);

        uint32_t flops = num_output_tiles * 2 * 32 * 32 * 32;
        __daisy_instrumentation_increment(region_id, "dram_bytes", reads + writes);
        __daisy_instrumentation_finalize(region_id);
    #endif

    tt::daisy::foam::copy_scalarField_from_device_dense_mat(buffer_pool, d_resVec, &result);

    tt::tt_metal::Finish(device->command_queue(0));

    Foam::Info << "Result: " << result << Foam::endl;

    tt::tt_metal::CloseDevice(device);

    return 0;
}