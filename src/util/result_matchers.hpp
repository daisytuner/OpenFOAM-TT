#pragma once

#include <scalarField.H>

namespace Foam::daisy {

constexpr float DEFAULT_SP_MATCHER_ATOL = 2e-10;
constexpr float DEFAULT_TF32_MATCHER_ATOL = 2e-5;
constexpr float DEFAULT_SP_MATCHER_RTOL = 2e-5;
constexpr float DEFAULT_TF32_MATCHER_RTOL = 2e-3;

#define TT_IMPL_LDU 0
#define TT_IMPL_DENSE 10
#define TT_IMPL_ELLPACK 20

#if defined(TT_IMPL) && (TT_IMPL == TT_IMPL_DENSE || TT_IMPL == TT_IMPL_ELLPACK)
constexpr float DEFAULT_MATCHER_ATOL = DEFAULT_TF32_MATCHER_ATOL;
constexpr float DEFAULT_MATCHER_RTOL = DEFAULT_TF32_MATCHER_RTOL;
#else
constexpr float DEFAULT_MATCHER_ATOL = DEFAULT_SP_MATCHER_ATOL;
constexpr float DEFAULT_MATCHER_RTOL = DEFAULT_SP_MATCHER_RTOL;
#endif

bool matches(const Foam::scalarField& a, const Foam::scalarField& b, float rtol = DEFAULT_MATCHER_RTOL, float atol = DEFAULT_MATCHER_ATOL);

}  // namespace Foam::daisy