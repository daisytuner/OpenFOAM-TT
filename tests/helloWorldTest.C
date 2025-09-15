#include <gtest/gtest.h>

#include "SquareMatrix.H"

TEST(HelloWorld, HelloWorldTest) {
    Foam::SquareMatrix<Foam::scalar> hmm
    {
        {-3.0, 10.0, -4.0},
        {2.0, 3.0, 10.0},
        {2.0, 6.0, 1.0}
    };

    // EXPECT_EQ(Foam::max(hmm), 10.0);
    // EXPECT_EQ(Foam::min(hmm), -4.0);

}
