#include "ldu_meta_cache.hpp"
#include "buffer_pool.hpp"
#include "device_transfers.hpp"
#include <iostream>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace tt::daisy::foam {

Foam::Ostream& operator<<(Foam::Ostream& os, const tt_ldu_meta& tt_meta) {
    os  << "tt_ldu_meta: " << Foam::endl
        << " cell_count: " << tt_meta.cell_count << Foam::endl
        << " sparse_count: " << tt_meta.sparse_count << Foam::endl
        << " addrs_on_device: " << tt_meta.addrs_on_device_ << Foam::endl
        << " upper_addr_start: " << tt_meta.upper_addrs_start_ << Foam::endl
        << " iface_map_start: " << tt_meta.iface_map_start_ << Foam::endl
        << " contents_on_device: " << tt_meta.contents_on_device_ << Foam::endl
        << " lower_contents_start: " << tt_meta.lower_contents_start_ << Foam::endl
        << " upper_contents_start: " << tt_meta.upper_contents_start_ << Foam::endl
        << " diag_zero: " << tt_meta.diag_zero << Foam::endl
        << " triang_zero: " << tt_meta.triang_zero << Foam::endl
        << " lower_contains_also_upper: " << tt_meta.lower_contains_also_upper << Foam::endl
        << " d_data_: " << (tt_meta.d_data_ ? "allocated" : "null") << Foam::endl
        << " d_addrs_: " << (tt_meta.d_addrs_ ? "allocated" : "null") << Foam::endl;
        
    return os;
}

std::unordered_map<const void*, tt_ldu_meta> ldu_tt_meta_map;

void verify_interfaces_noop(const Foam::lduInterfaceFieldPtrsList& interfaces) {
    forAll(interfaces, i) {
        if (interfaces.set(i)) {
            throw std::runtime_error("Interface operations not supported with TT");
        }
    }
}

void clear_tt_meta(const void* key, bool clear_addrs, bool clear_contents) {

    auto it = ldu_tt_meta_map.find(key);
    if (it != ldu_tt_meta_map.end()) {
        auto& meta = it->second;
        if (clear_addrs && meta.addrs_on_device_) {
            #ifdef TRACY_ENABLE
            ZoneScopedN("TT Meta clear addr");
            #endif
            meta.addrs_on_device_ = false;
            std::cout << "Cleared TT meta addrs for " << key << std::endl;
        }
        if (clear_contents && meta.contents_on_device_) {
            #ifdef TRACY_ENABLE
            ZoneScopedN("TT Meta clear contents");
            #endif
            meta.contents_on_device_ = false;
            meta.dense_on_device_ = false;
            meta.ellpack_on_device_ = false;
        }
    } // never uploaded to begin with
}

}  // namespace tt::daisy::foam