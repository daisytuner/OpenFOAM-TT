
#include "result_matchers.hpp"

namespace Foam::daisy {

bool matches(const Foam::scalarField& a, const Foam::scalarField& b, float rtol, float atol) {
    if (a.size() != b.size()) {
        return false;
    }
    
    for (int i = 0; i < a.size(); ++i) {
        auto diff = std::abs(a[i] - b[i]);
        auto tol = atol + rtol * std::abs(b[i]);
        if (diff > tol) {
            std::cout << "Mismatch at index " << i << ": a=" << a[i] << ", b=" << b[i]
                      << ", diff=" << diff << ", tol=" << tol << std::endl;
            return false;
        }
    }
    return true;
}

}  // namespace Foam::daisy