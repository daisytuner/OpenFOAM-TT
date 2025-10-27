#ifdef ENABLE_DAISY_RTL
#include <daisy_rtl/daisy_rtl.h>
#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <tt-metalium/host_api.hpp>
#include "tt-metalium/tt_metal_profiler.hpp"

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

int main(int argc, char* argv[]) {

    int Nx, Ny;

    // Parse command line arguments
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <Nx> <Ny>" << std::endl;
        std::cerr << "  Nx: Number of cells in x direction" << std::endl;
        std::cerr << "  Ny: Number of cells in y direction" << std::endl;
        std::cerr << "Continuing with default 5x5 grid." << std::endl;
        Nx = 5;
        Ny = 5;
    }
    else {

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
    // For LDU format: lower has row > col, upper has row < col
    for (Foam::label j = 0; j < Ny; ++j) {
        for (Foam::label i = 0; i < Nx; ++i) {
            Foam::label cell = i + j * Nx;

            // East neighbor (i+1) - upper triangle (cell < neighbor)
            if (i < Nx - 1) {
                Foam::label neighbor = (i + 1) + j * Nx;
                addr_upper[idx] = neighbor;  // row (higher index)
                addr_lower[idx] = cell;      // col (lower index)
                ++idx;
            }

            // North neighbor (j+1) - upper triangle (cell < neighbor)
            if (j < Ny - 1) {
                Foam::label neighbor = i + (j + 1) * Nx;
                addr_upper[idx] = neighbor;  // row (higher index)
                addr_lower[idx] = cell;      // col (lower index)
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


     #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata1 = {
        .file_name = "bench_dense_Amul_real.cpp",
        .function_name = "copy_ldu_to_dense",
        .line_begin = 154,
        .line_end = 158,
        .column_begin = 0,
        .column_end = 0,
        .element_type = "h2d_transfer",
        .target_type = "TENSTORRENT",
        .region_uuid = "copy_ldu_to_dense"
    };
    unsigned long long region_id1 = __daisy_instrumentation_init(&metadata1, __DAISY_EVENT_SET_NONE);
    __daisy_instrumentation_enter(region_id1);
    #endif

    // copying starts

    tt::daisy::foam::copy_ldu_to_dense(device, tt_meta_a, &lduA);

    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_exit(region_id1);
        __daisy_instrumentation_increment(region_id1, "pcie_bytes", 1);
        __daisy_instrumentation_finalize(region_id1);
    #endif


    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata2 = {
        .file_name = "bench_dense_Amul_real.cpp",
        .function_name = "copy_scalarfield",
        .line_begin = 178,
        .line_end = 182,
        .column_begin = 0,
        .column_end = 0,
        .element_type = "h2d_transfer",
        .target_type = "TENSTORRENT",
        .region_uuid = "copy_scalarfield"
    };
    unsigned long long region_id2 = __daisy_instrumentation_init(&metadata2, __DAISY_EVENT_SET_NONE);
    __daisy_instrumentation_enter(region_id2);
    #endif

    auto& d_inVec = tt::daisy::foam::copy_scalarField_to_device_as_dense_mat(buffer_pool, inVec);
    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_increment(region_id2, "pcie_bytes", 1);
        __daisy_instrumentation_finalize(region_id2);
    #endif

    auto& d_resWarmup = buffer_pool.allocateBuffer(d_inVec.buffer->size(),tile_size);
    auto& d_resVec = buffer_pool.allocateBuffer(d_inVec.buffer->size(), tile_size);

    tt_launch_dense_matMul(
        device,
        *tt_meta_a.d_dense_,
        *d_inVec.buffer,
        *d_resWarmup.buffer,
        cells_aligned,
        32,
        cells_aligned,
        1,
        false,
        kernel_dir
    );

    tt::tt_metal::Finish(device->command_queue(0));
    tt::tt_metal::detail::ReadDeviceProfilerResults(device);

    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata3 = {
        .file_name = "bench_ldu_Amul_real.cpp",
        .function_name = "tt_launch_dense_matMul",
        .line_begin = 218,
        .line_end = 235,
        .column_begin = 0,
        .column_end = 0,
        .target_type = "TENSTORRENT",
        .region_uuid = "tt_launch_dense_matMul"
    };
    unsigned long long region_id3 = __daisy_instrumentation_init(&metadata3, __DAISY_EVENT_SET_NONE);
    __daisy_instrumentation_enter(region_id3);
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
        __daisy_instrumentation_exit(region_id3);
        uint64_t M = cells_aligned;
        uint64_t N = 32;
        uint64_t K = cells_aligned;
        uint64_t Kt = K / tt::constants::TILE_WIDTH;
        uint64_t num_output_tiles = (M * N) / tt::constants::TILE_HW;
        uint64_t num_tiles = num_output_tiles;

        uint64_t reads = num_output_tiles * Kt  * (2 * tt::constants::TILE_HW) * sizeof(float);
        uint64_t writes = num_tiles * tt::constants::TILE_HW;
        uint64_t bytes = reads + writes;

        uint64_t flops = num_output_tiles * Kt * 2 * tt::constants::TILE_HW * tt::constants::TILE_WIDTH;
        __daisy_instrumentation_increment(region_id3, "flop", flops); // ~ 2 * M * N * K
        __daisy_instrumentation_increment(region_id3, "dram_bytes", bytes);
        __daisy_instrumentation_finalize(region_id3);
    #endif

    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata4 = {
        .file_name = "bench_dense_Amul_real.cpp",
        .function_name = "copy_scalarField_from_device_dense_mat",
        .line_begin = 267,
        .line_end = 169,
        .column_begin = 0,
        .column_end = 0,
        .element_type = "d2h_transfer",
        .target_type = "TENSTORRENT",
        .region_uuid = "copy_scalarField_from_device_dense_mat"
    };
        unsigned long long region_id4 = __daisy_instrumentation_init(&metadata4, __DAISY_EVENT_SET_NONE);
        __daisy_instrumentation_enter(region_id4);
    #endif

    tt::daisy::foam::copy_scalarField_from_device_dense_mat(buffer_pool, d_resVec, &result);

    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_exit(region_id4);
        __daisy_instrumentation_increment(region_id4, "pcie_bytes", 1);
        __daisy_instrumentation_finalize(region_id4);
    #endif


    Foam::scalarField expected(cells);

    refAmul(lduA, expected, inVec);

    // Foam::Info << "Result: " << result << Foam::endl;

    if (!Foam::daisy::matches(result, expected, Foam::daisy::DEFAULT_TF32_MATCHER_RTOL, Foam::daisy::DEFAULT_TF32_MATCHER_ATOL)) {
        Foam::SeriousError << "FAIL Expected: " << expected << Foam::endl;
        Foam::Info << "Result: " << result << Foam::endl;
    } else {
        Foam::Info << "PASS" << Foam::endl;
        // Foam::Info << "Result: " << result << Foam::endl;
        // Foam::Info << "Expected: " << expected << Foam::endl;
        // Foam::Info << "Mat: " << lduA << Foam::endl;
    }

    tt::tt_metal::CloseDevice(device);

    return 0;
}