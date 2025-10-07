
#include "result_matchers.hpp"

namespace Foam::daisy {

bool matches(const Foam::scalarField& a, const Foam::scalarField& b, float tol) {
    if (a.size() != b.size()) {
        return false;
    }
    for (int i = 0; i < a.size(); ++i) {
        if (std::abs(a[i] - b[i]) > tol) {
            return false;
        }
    }
    return true;
}

}  // namespace Foam::daisy