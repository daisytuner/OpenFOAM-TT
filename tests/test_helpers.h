#pragma once

#define EXPECT_TRUE(x) if (!(x)) { Foam::Info << "Expectation failed: " #x << "\n"; return 1; }
#define EXPECT_FALSE(x) if (x) { Foam::Info << "Expectation failed: " #x << "\n"; return 1; }
#define EXPECT_FLOAT_EQ(x, y) if (std::fabs((x) - (y)) > 1e-5) { Foam::Info << "Expectation failed: " #x " == " #y << " (" << (x) << " != " << (y) << ")" << "\n"; return 1; }
#define EXPECT_DOUBLE_EQ(x, y) if (std::fabs((x) - (y)) > 1e-12) { Foam::Info << "Expectation failed: " #x " == " #y << " (" << (x) << " != " << (y) << ")" << "\n"; return 1; }