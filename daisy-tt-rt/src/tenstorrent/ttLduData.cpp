
#include "ttLduData.hpp"

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif


namespace tt::daisy {

tt_ldu_meta::~tt_ldu_meta() {
    if (ellpack_addr_) {
        delete[] ellpack_addr_;
        ellpack_addr_ = nullptr;
    }
}

}  // namespace tt::daisy
