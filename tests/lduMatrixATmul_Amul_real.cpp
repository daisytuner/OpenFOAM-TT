#include "SquareMatrix.H"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"
#include "IOstreams.H"
#include "../tt-bench-harness/ref_Amul.hpp"
#include "scalarField.H"
#include "result_matchers.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
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

    std::cout << "Created lduPrimitiveMesh" << std::endl;

    Foam::lduMatrix lduA(mesh);

    // Fill values
    lduA.diag() = 4.0;  // interior cells have 4 neighbors
    lduA.lower() = -1.0;
    lduA.upper() = -1.0;

    std::cout << "Created lduMatrix" << std::endl;

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

    std::cout << "Filled lduMatrix values" << std::endl;
    
    bool allCorrect = true;

    for (int i = 0; i < 1000; ++i) {
        // Construct a vector to multiply
        Foam::scalarField vec(cells, 2.0);

        Foam::direction cmpt = Foam::direction(0);

        // Kernel

        Foam::scalarField result(cells, 0.0);
        const Foam::tmp<Foam::scalarField> tvec(vec);
        lduA.Amul(
            result,
            tvec,
            Foam::FieldField<Foam::Field, Foam::scalar>(0),
            Foam::lduInterfaceFieldPtrsList(0),
            cmpt
        );

        Foam::scalarField refResult(cells);
        refAmul(lduA, refResult, tvec);
        if (!Foam::daisy::matches(result, refResult, Foam::daisy::DEFAULT_TF32_MATCHER_RTOL, Foam::daisy::DEFAULT_TF32_MATCHER_ATOL)) {
            Foam::SeriousError << "FAIL Expected: " << refResult << Foam::endl;
            Foam::Info << "Result: " << result << Foam::endl;
        } else {
            Foam::Info << "PASS" << Foam::endl;
            // Foam::Info << "Result: " << result << Foam::endl;
            // Foam::Info << "Expected: " << refResult << Foam::endl;
            // Foam::Info << "Mat: " << lduA << Foam::endl;
        }
    }

    // if (tt::DevicePool::is_initialized()) {
    //     auto& inst = tt::DevicePool::instance();
    //     if (inst.is_device_active(0)) {
    //         std::cerr << "Closing device 0 before exiting test." << std::endl;
    //         inst.close_device(0);
    //     }
    // }

    if (allCorrect) {
        Foam::Info << "All results correct!" << Foam::endl;
        return 0;
    } else {
        return 1;
    }
}
