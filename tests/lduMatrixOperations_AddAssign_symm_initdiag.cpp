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

    b_matrix.diag() = Foam::scalarField({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                              11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0});

    EXPECT_FALSE(a_matrix.hasDiag());
    EXPECT_FALSE(a_matrix.hasUpper());
    EXPECT_FALSE(a_matrix.hasLower());
    EXPECT_TRUE(b_matrix.hasDiag());
    EXPECT_FALSE(b_matrix.hasUpper());
    EXPECT_FALSE(b_matrix.hasLower());

    a_matrix += b_matrix;

    EXPECT_TRUE(a_matrix.hasDiag());
    EXPECT_FALSE(a_matrix.hasUpper());
    EXPECT_FALSE(a_matrix.hasLower());
    EXPECT_TRUE(b_matrix.hasDiag());
    EXPECT_FALSE(b_matrix.hasUpper());
    EXPECT_FALSE(b_matrix.hasLower());

    for (int i = 0; i < cells; ++i) {
        auto expected_diag = 1.0f*(i+1);
        EXPECT_FLOAT_EQ(a_matrix.diag()[i], expected_diag);
    }

    return 0;
}
