#include <gtest/gtest.h>

#include "SquareMatrix.H"
#include "label.H"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"
#include "scalarList.H"

TEST(Tenstorrent_Kernels, LduMatAddAssign_asymm) {
    Foam::label cells = 20;
    // Diagonal coefficients
    Foam::scalarField a_diag({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                              11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0});

    // Lower and upper coefficients (off-diagonal)
    Foam::labelList addr_upper(190); // upper  
    Foam::labelList addr_lower(190); // lower

    Foam::scalarList a_upper(190);
    Foam::scalarList a_lower(190);
    Foam::scalarList b_upper(190);
    Foam::scalarList b_lower(190);

    // Walk the upper triangle of a square matrix (excluding diagonal)
    int idx = 0;
    for (Foam::label row = 0; row < cells; ++row) {
        for (Foam::label col = row + 1; col < cells; ++col) {
            addr_upper[idx] = col;
            addr_lower[idx] = row;


            auto a_val = (row == 0 && (col == 10 || col == 19)? 3.0 : 0.0);
            a_upper[idx] = a_val;
            a_lower[idx] = a_val;

            auto b_val = 1.0;
            b_upper[idx] = b_val;
            b_lower[idx] = b_val;

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

    a_matrix.diag() = a_diag;
    a_matrix.lower() = a_lower;
    a_matrix.upper() = a_upper;

    Foam::lduMatrix b_matrix(mesh);
    b_matrix.diag() = Foam::scalarField(20, 1.0);
    b_matrix.lower() = b_lower;
    b_matrix.upper() = b_upper;

    
    a_matrix += b_matrix;

    for (int i = 0; i < cells; ++i) {
        auto expected_diag = 1.0f * (i+1) + 1.0f;
        EXPECT_FLOAT_EQ(a_matrix.diag()[i], expected_diag) << " at diag index " << i;
    }

    idx = 0;
    for (Foam::label row = 0; row < cells; ++row) {
        for (Foam::label col = row + 1; col < cells; ++col) {
            auto org_a_val = (row == 0 && (col == 10 || col == 19)? 3.0 : 0.0);
            EXPECT_FLOAT_EQ(a_matrix.upper()[idx], org_a_val + 1.0f) << " at upper index " << idx << " (" << row << "," << col << ")";
            EXPECT_FLOAT_EQ(a_matrix.lower()[idx], org_a_val + 1.0f) << " at lower index " << idx << " (" << row << "," << col << ")";

            ++idx;
        }
    }
}

TEST(Tenstorrent_Kernels, LduMatAddAssign_symm) {
    Foam::label cells = 20;
    // Diagonal coefficients
    Foam::scalarField a_diag({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                              11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0});

    // Lower and upper coefficients (off-diagonal)
    Foam::labelList addr_upper(190); // upper  
    Foam::labelList addr_lower(190); // lower

    Foam::scalarList a_upper(190);
    Foam::scalarList b_upper(190);

    // Walk the upper triangle of a square matrix (excluding diagonal)
    int idx = 0;
    for (Foam::label row = 0; row < cells; ++row) {
        for (Foam::label col = row + 1; col < cells; ++col) {
            addr_upper[idx] = col;
            addr_lower[idx] = row;


            auto a_val = (row == 0 && (col == 10 || col == 19)? 3.0 : 0.0);
            a_upper[idx] = a_val;

            auto b_val = 1.0;
            b_upper[idx] = b_val;

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

    a_matrix.diag() = a_diag;
    a_matrix.upper() = a_upper;

    Foam::lduMatrix b_matrix(mesh);
    b_matrix.diag() = Foam::scalarField(20, 1.0);
    b_matrix.upper() = b_upper;

    EXPECT_TRUE(a_matrix.symmetric());
    EXPECT_TRUE(a_matrix.hasUpper());
    EXPECT_FALSE(a_matrix.hasLower());
    EXPECT_TRUE(b_matrix.symmetric());
    EXPECT_TRUE(b_matrix.hasUpper());
    EXPECT_FALSE(b_matrix.hasLower());

    
    a_matrix += b_matrix;

    EXPECT_TRUE(a_matrix.symmetric());
    EXPECT_TRUE(a_matrix.hasUpper());
    EXPECT_FALSE(a_matrix.hasLower());
    EXPECT_TRUE(b_matrix.symmetric());
    EXPECT_TRUE(b_matrix.hasUpper());
    EXPECT_FALSE(b_matrix.hasLower());

    for (int i = 0; i < cells; ++i) {
        auto expected_diag = 1.0f * (i+1) + 1.0f;
        EXPECT_FLOAT_EQ(a_matrix.diag()[i], expected_diag) << " at diag index " << i;
    }

    idx = 0;
    for (Foam::label row = 0; row < cells; ++row) {
        for (Foam::label col = row + 1; col < cells; ++col) {
            auto org_a_val = (row == 0 && (col == 10 || col == 19)? 3.0 : 0.0);
            EXPECT_FLOAT_EQ(a_matrix.upper()[idx], org_a_val + 1.0f) << " at upper index " << idx << " (" << row << "," << col << ")";

            ++idx;
        }
    }
}

TEST(Tenstorrent_Kernels, LduMatAddAssign_symm_expand) {
    Foam::label cells = 20;
    // Diagonal coefficients
    Foam::scalarField a_diag({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                              11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0});

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


            auto a_val = (row == 0 && (col == 10 || col == 19)? 3.0f : 0.0f);
            a_upper[idx] = a_val;

            auto b_val = 1.0f;
            b_upper[idx] = b_val;
            b_lower[idx] = b_val;

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

    a_matrix.diag() = a_diag;
    a_matrix.upper() = a_upper;

    Foam::lduMatrix b_matrix(mesh);
    b_matrix.diag() = Foam::scalarField(20, 1.0);
    b_matrix.upper() = b_upper;
    b_matrix.lower() = b_lower;

    EXPECT_TRUE(a_matrix.symmetric());
    EXPECT_TRUE(a_matrix.hasUpper());
    EXPECT_FALSE(a_matrix.hasLower());
    EXPECT_TRUE(b_matrix.asymmetric());
    EXPECT_TRUE(b_matrix.hasUpper());
    EXPECT_TRUE(b_matrix.hasLower());

    
    a_matrix += b_matrix;

    EXPECT_TRUE(a_matrix.asymmetric());
    EXPECT_TRUE(a_matrix.hasUpper());
    EXPECT_TRUE(a_matrix.hasLower());
    EXPECT_TRUE(b_matrix.asymmetric());
    EXPECT_TRUE(b_matrix.hasUpper());
    EXPECT_TRUE(b_matrix.hasLower());

    for (int i = 0; i < cells; ++i) {
        auto expected_diag = 1.0f * (i+1) + 1.0f;
        EXPECT_FLOAT_EQ(a_matrix.diag()[i], expected_diag) << " at diag index " << i;
    }

    idx = 0;
    for (Foam::label row = 0; row < cells; ++row) {
        for (Foam::label col = row + 1; col < cells; ++col) {
            auto org_a_val = (row == 0 && (col == 10 || col == 19)? 3.0f : 0.0f);
            EXPECT_FLOAT_EQ(a_matrix.upper()[idx], org_a_val + 1.0f) << " at upper index " << idx << " (" << row << "," << col << ")";
            EXPECT_FLOAT_EQ(a_matrix.lower()[idx], org_a_val + 1.0f) << " at lower index " << idx << " (" << row << "," << col << ")";   

            ++idx;
        }
    }
}

TEST(Tenstorrent_Kernels, LduMatAddAssign_does_not_init_without_input) {
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


}

TEST(Tenstorrent_Kernels, LduMatAddAssign_will_init_dest_diag) {
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
        EXPECT_FLOAT_EQ(a_matrix.diag()[i], expected_diag) << " at diag index " << i;
    }
}