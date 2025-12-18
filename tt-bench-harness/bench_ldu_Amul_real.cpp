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
#include "kernel_launcher.hpp"
#include "ldu_meta_cache.hpp"
#include "device_transfers.hpp"

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

    auto& k = tt::daisy::foam::require_kernel_launcher();

     #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata1 = {
        .file_name = "bench_ldu_Amul_real.cpp",
        .function_name = "main",
        .line_begin = 154,
        .line_end = 158,
        .column_begin = 0,
        .column_end = 0,
        .target_type = "TENSTORRENT",
        .region_uuid = "ensure_lduMat_on_device"
    };
        unsigned long long region_id1 = __daisy_instrumentation_init(&metadata1, __DAISY_EVENT_SET_NONE);
        __daisy_instrumentation_enter(region_id1);
    #endif

    // copying starts
    auto [tt_meta, h2d_dat, h2d_mesh] = tt::daisy::foam::ensure_lduMat_on_device(buffer_pool, &lduA);

    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_exit(region_id1);
        __daisy_instrumentation_increment(region_id1, "flop", 0);
        __daisy_instrumentation_increment(region_id1, "dram_bytes", 1);
        __daisy_instrumentation_finalize(region_id1);
    #endif


    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata2 = {
        .file_name = "bench_dense_Amul_real.cpp",
        .function_name = "main",
        .line_begin = 178,
        .line_end = 182,
        .column_begin = 0,
        .column_end = 0,
        .target_type = "TENSTORRENT",
        .region_uuid = "copy_ldu_to_dense"
    };
        unsigned long long region_id2 = __daisy_instrumentation_init(&metadata2, __DAISY_EVENT_SET_NONE);
        __daisy_instrumentation_enter(region_id2);
    #endif

    auto& d_inVec = tt::daisy::foam::copy_scalarField_to_device(buffer_pool, inVec);

    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_exit(region_id2);
        __daisy_instrumentation_increment(region_id2, "flop", 0);
        __daisy_instrumentation_increment(region_id2, "dram_bytes", 1);
        __daisy_instrumentation_finalize(region_id2);
    #endif

    auto& d_resWarmup = buffer_pool.allocateBuffer(d_inVec.buffer->size(), tile_size);
    auto& d_resVec = buffer_pool.allocateBuffer(d_inVec.buffer->size(), tile_size);

    tt::daisy::foam::tt_compute_amul(k, tt_meta, d_inVec, d_resWarmup);

    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata3 = {
        .file_name = "bench_ldu_Amul_real.cpp",
        .function_name = "main",
        .line_begin = 218,
        .line_end = 235,
        .column_begin = 0,
        .column_end = 0,
        .element_type = "map",
        .target_type = "TENSTORRENT",
        .region_uuid = "tt_launch_dense_matMul"
    };

    unsigned long long region_id3 = __daisy_instrumentation_init(&metadata3, __DAISY_EVENT_SET_NONE);
    __daisy_instrumentation_enter(region_id3);
    #endif

    tt::daisy::foam::tt_compute_amul(k, tt_meta, d_inVec, d_resVec);

    tt::tt_metal::Finish(device->command_queue(0));


    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_exit(region_id3);
            uint32_t page_size = 1024;
            uint32_t page_count_faces = (tt_meta.iface_map_start_ + page_size/4 + page_size/4) / (page_size / 4);
            uint32_t page_count_offdiagonal = (tt_meta.upper_contents_start_+ tt_meta.sparse_count + page_size/4 -1) / (page_size / 4);
            uint32_t page_count_diagonal = (tt_meta.cell_count + page_size/4 -1)/ (page_size / 4);
            uint32_t reads = page_size* (page_count_faces + page_count_diagonal) * sizeof(float) + page_size * page_count_offdiagonal * sizeof(int);
            uint32_t writes = page_size * page_count_diagonal * sizeof(float);
        __daisy_instrumentation_increment(region_id3, "flop", 4 * tt_meta.sparse_count);
        __daisy_instrumentation_increment(region_id3, "dram_bytes", reads + writes);
        __daisy_instrumentation_finalize(region_id3);
    #endif

    #ifdef ENABLE_DAISY_RTL
    __daisy_metadata_t metadata4 = {
        .file_name = "bench_dense_Amul_real.cpp",
        .function_name = "main",
        .line_begin = 267,
        .line_end = 269,
        .column_begin = 0,
        .column_end = 0,
        .target_type = "TENSTORRENT",
        .region_uuid = "copy_scalarField_from_device"
    };
    unsigned long long region_id4 = __daisy_instrumentation_init(&metadata4, __DAISY_EVENT_SET_NONE);
    __daisy_instrumentation_enter(region_id4);
    #endif

    tt::daisy::foam::copy_scalarField_from_device(buffer_pool, d_resVec, &result);

    tt::tt_metal::Finish(device->command_queue(0));

    #ifdef ENABLE_DAISY_RTL
        __daisy_instrumentation_exit(region_id4);
        __daisy_instrumentation_increment(region_id4, "flop", 0);
        __daisy_instrumentation_increment(region_id4, "dram_bytes", 1);
        __daisy_instrumentation_finalize(region_id4);
    #endif

    Foam::Info << "Result: " << result << Foam::endl;

    tt::tt_metal::CloseDevice(device);

    return 0;
}