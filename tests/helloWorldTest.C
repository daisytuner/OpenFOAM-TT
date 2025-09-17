#include <gtest/gtest.h>

#include "SquareMatrix.H"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"

TEST(HelloWorld, HelloWorldTest) {
    Foam::SquareMatrix<Foam::scalar> hmm
    {
        {-3.0, 10.0, -4.0},
        {2.0, 3.0, 10.0},
        {2.0, 6.0, 1.0}
    };

    EXPECT_EQ(Foam::max(hmm), 10.0);
    EXPECT_EQ(Foam::min(hmm), -4.0);

}

TEST(HelloWorld, Mul) {
    // Number of cells
    Foam::label nCells = 3;

    // Diagonal coefficients
    Foam::scalarField diag(nCells, 2.0);

    // Lower and upper coefficients (off-diagonal)
    Foam::labelList upper{2, 2}; // upper  
    Foam::labelList lower{0, 1}; // lower
    // both form the coordinates of the off-diagonal entries together. For lower entries upperAddr,lowerAddr. For upper entries lowerAddr,upperAddr
    // position of non-zero elems in upper and lower triangle ARE MIRRORED. They model interaction between neighboring cells of the simulation
    // The connections between neughboring cells are called "faces" and have a native order

    // -> (1,1) (2,1)
    // -> (1,1) (1,2)

    // lduAddressing
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
    matrix.lower() = Foam::scalarField{ 2, -1.0};
    matrix.upper() = Foam::scalarField{ 2, -2.0};

    /* =>
    {  2.0,  0.0, -2.0},
    {  0.0,  2.0, -2.0},
    { -1.0, -1.0,  2.0}
    */

    Foam::Info << "lduMat: " << matrix << Foam::endl;

    Foam::scalarField result(nCells, 0.0);

    matrix.sumA(
        result,
        Foam::FieldField<Foam::Field, Foam::scalar>(0),
        Foam::lduInterfaceFieldPtrsList(0)
    );

    Foam::Info << "sumA: " << result << Foam::endl;

    // Test vector
    Foam::scalarField x({1.0, 2.0, 3.0});

    Foam::Info << "Vector: " << x << Foam::endl;

    // Matrix-vector multiplication
    matrix.Amul(
        result,
        x,
        Foam::FieldField<Foam::Field, Foam::scalar>(0),
        Foam::lduInterfaceFieldPtrsList(0),
        ' '
    );

    // Print result
    Foam::Info << "Result: " << result << Foam::endl;

}