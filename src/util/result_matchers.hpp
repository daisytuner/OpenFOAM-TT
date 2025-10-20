#pragma once

#include <scalarField.H>

namespace Foam::daisy {

constexpr float DEFAULT_SP_MATCHER_TOL = 1e-10;
constexpr float DEFAULT_TF32_MATCHER_TOL = 1e-7;

#define TT_IMPL_LDU 0
#define TT_IMPL_DENSE 10
#define TT_IMPL_ELLPACK 20

#if defined(TT_IMPL) && TT_IMPL == TT_IMPL_DENSE
constexpr float DEFAULT_MATCHER_TOL = DEFAULT_TF32_MATCHER_TOL;
#else
constexpr float DEFAULT_MATCHER_TOL = DEFAULT_SP_MATCHER_TOL;
#endif

bool matches(const Foam::scalarField& a, const Foam::scalarField& b, float tol = DEFAULT_MATCHER_TOL);

}  // namespace Foam::daisy