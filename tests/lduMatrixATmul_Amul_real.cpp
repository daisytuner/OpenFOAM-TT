#include "SquareMatrix.H"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"
#include "IOstreams.H"

int main(int argc, char* argv[])
{
/    int Nx, Ny;

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

        // Construct a vector to multiply"

        // Construct a vector to multiply
    Foam::scalarField vec(cells, 2.0);

    Foam::direction cmpt = Foam::direction(0);

    // Kernel

    Foam::scalarField result(cells, 0.0);
    lduA.Amul(
        result,
        vec,
        Foam::FieldField<Foam::Field, Foam::scalar>(0),
        Foam::lduInterfaceFieldPtrsList(0),
        cmpt
    );

    // Check result
    Foam::Info << "Amul: " << result << Foam::endl;

    // Verify results - for a finite difference discretization with vec=2.0
    // Interior cells: diag=4, 4 off-diag connections -> result = 4*2 + 4*(-1)*2 = 0
    // Boundary cells: varies based on number of neighbors
    bool allCorrect = true;
    for (Foam::label j = 0; j < Ny; ++j) {
        for (Foam::label i = 0; i < Nx; ++i) {
            Foam::label cell = i + j * Nx;
            Foam::scalar numNeighbors = 0.0;
            if (i > 0) numNeighbors += 1.0;
            if (i < Nx - 1) numNeighbors += 1.0;
            if (j > 0) numNeighbors += 1.0;
            if (j < Ny - 1) numNeighbors += 1.0;

            Foam::scalar expected = lduA.diag()[cell] * 2.0 - numNeighbors * 2.0;

            if (Foam::mag(result[cell] - expected) > 1e-10) {
                Foam::Info << "Error: result[" << cell << "] = " << result[cell]
                           << ", expected " << expected << Foam::endl;
                allCorrect = false;
            }
        }
    }

    if (allCorrect) {
        Foam::Info << "All results correct!" << Foam::endl;
        return 0;
    } else {
        return 1;
    }
}
