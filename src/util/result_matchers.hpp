#pragma once

#include <scalarField.H>

namespace Foam::daisy {

const float DEFAULT_SP_MATCHER_TOL = 1e-10;

bool matches(const Foam::scalarField& a, const Foam::scalarField& b, float tol = DEFAULT_SP_MATCHER_TOL);

}  // namespace Foam::daisy