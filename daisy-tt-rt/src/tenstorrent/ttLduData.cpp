
#include "ttLduData.hpp"

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif


namespace tt::daisy {

tt_ldu_meta::~tt_ldu_meta() {
    if (ellpack_addr_) {
        delete[] ellpack_addr_;
        ellpack_addr_ = nullptr;
        delete[] ellpack_first_col_per_tile_;
        ellpack_first_col_per_tile_ = nullptr;
        delete[] ellpack_last_col_per_tile_;
        ellpack_last_col_per_tile_ = nullptr;
    }
}

}  // namespace tt::daisy
