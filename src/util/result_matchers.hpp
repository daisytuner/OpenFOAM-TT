#pragma once

#include <scalarField.H>
#include <tt_impls.hpp>

namespace Foam::daisy {

constexpr float DEFAULT_SP_MATCHER_TOL = 1e-10;
constexpr float DEFAULT_TF32_MATCHER_TOL = 1e-7;

#if TT_IMPL == TT_IMPL_DENSE
constexpr float DEFAULT_MATCHER_TOL = DEFAULT_TF32_MATCHER_TOL;
#else
constexpr float DEFAULT_MATCHER_TOL = DEFAULT_SP_MATCHER_TOL;
#endif

bool matches(const Foam::scalarField& a, const Foam::scalarField& b, float tol = DEFAULT_MATCHER_TOL);

}  // namespace Foam::daisy