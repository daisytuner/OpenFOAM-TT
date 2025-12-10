#include "SquareMatrix.H"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"
#include "ldu_meta_cache.hpp"

__attribute__((noinline))
void kernel(Foam::label& nCells, Foam::lduMatrix& matrix, Foam::scalarField& vec, Foam::FieldField<Foam::Field, Foam::scalar>& interfaceBouCoeffs, Foam::lduInterfaceFieldPtrsList& interfaces, Foam::direction& cmpt, Foam::scalarField& result1, Foam::scalarField& result2) {
    // Kernel 1
    matrix.Amul(
        result1,
        vec,
        interfaceBouCoeffs,
        interfaces,
        cmpt
    );

    // Kernel 2
    matrix.Amul(
        result2,
        result1,
        interfaceBouCoeffs,
        interfaces,
        cmpt
    );
}

int main()
{
    // Create a simple 3x3 mesh
    Foam::label nCells = 3;
    Foam::scalarField diag(nCells, 2.0);
    Foam::labelList upper{2, 2};
    Foam::labelList lower{0, 1};
    Foam::lduPrimitiveMesh mesh(
        nCells,
        lower,
        upper,
        0, // comm
        false
    );

    // Construct the lduMatrix
    Foam::lduMatrix matrix(mesh);
    matrix.diag() = diag;
    matrix.lower() = Foam::scalarField{ 2, 1.0};
    matrix.upper() = Foam::scalarField{ 2, 2.0};
    
    /* =>
    {  2.0,  0.0,  2.0},
    {  0.0,  2.0,  2.0},
    {  1.0,  1.0,  2.0}
    */

    // Construct a vector to multiply
    Foam::scalarField vec(nCells, 2.0); // {2.0, 2.0, 2.0}

    Foam::direction cmpt = Foam::direction(0);

    Foam::scalarField result1(nCells, 0.0);
    Foam::scalarField result2(nCells, 0.0);

    Foam::FieldField<Foam::Field, Foam::scalar> interfaceBouCoeffs(0);
    Foam::lduInterfaceFieldPtrsList interfaces(0);

    kernel(nCells, matrix, vec, interfaceBouCoeffs, interfaces, cmpt, result1, result2);

    // Check result1
    Foam::Info << "Amul: " << result1 << Foam::endl;

    for (size_t i = 0; i < nCells; ++i)
    {
        if (result1[i] != 8.0) {
            Foam::Info << "Error: result1[" << i << "] = " << result1[i]
                       << ", expected 8.0" << Foam::endl;
            return 1;
        }
    }

    // Check result2
    Foam::Info << "Amul: " << result2 << Foam::endl;

    for (size_t i = 0; i < nCells; ++i)
    {
        if (result2[i] != 32.0) {
            Foam::Info << "Error: result2[" << i << "] = " << result2[i]
                       << ", expected 32.0" << Foam::endl;
            return 1;
        }
    }

    tt::daisy::foam::ldu_tt_meta_map.clear();

    return 0;
}
