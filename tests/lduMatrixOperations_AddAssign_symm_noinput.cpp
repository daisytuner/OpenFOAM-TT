#include "SquareMatrix.H"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"

#include "test_helpers.h"

int main()
{
    Foam::label cells = 20;
    // Diagonal coefficients
    Foam::scalarField a_diag(cells);

    // Lower and upper coefficients (off-diagonal)
    Foam::labelList addr_upper(190); // upper  
    Foam::labelList addr_lower(190); // lower

    Foam::scalarList a_upper(190);
    Foam::scalarList b_upper(190);
    Foam::scalarList b_lower(190);

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
        false
    );


    Foam::lduMatrix a_matrix(mesh);

    Foam::lduMatrix b_matrix(mesh);

    EXPECT_FALSE(a_matrix.hasDiag());
    EXPECT_FALSE(a_matrix.hasUpper());
    EXPECT_FALSE(a_matrix.hasLower());
    EXPECT_FALSE(b_matrix.hasDiag());
    EXPECT_FALSE(b_matrix.hasUpper());
    EXPECT_FALSE(b_matrix.hasLower());

    a_matrix += b_matrix;

    EXPECT_FALSE(a_matrix.hasDiag());
    EXPECT_FALSE(a_matrix.hasUpper());
    EXPECT_FALSE(a_matrix.hasLower());
    EXPECT_FALSE(b_matrix.hasDiag());
    EXPECT_FALSE(b_matrix.hasUpper());
    EXPECT_FALSE(b_matrix.hasLower());

    return 0;
}
