#include "SquareMatrix.H"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"

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
    {  2.0,  0.0, -2.0},
    {  0.0,  2.0, -2.0},
    { -1.0, -1.0,  2.0}
    */

    // Kernel

    Foam::scalarField result(nCells, 0.0);
    matrix.sumA(
        result,
        Foam::FieldField<Foam::Field, Foam::scalar>(0),
        Foam::lduInterfaceFieldPtrsList(0)
    );

    // Check result
    Foam::Info << "sumA: " << result << Foam::endl;

    for (size_t i = 0; i < nCells; ++i)
    {
        if (result[i] != 4.0) {
            Foam::Info << "Error: result[" << i << "] = " << result[i]
                       << ", expected 4.0" << Foam::endl;
            return 1;
        }
    }

    return 0;
}