
#include "ttLduData.hpp"
#include "device_transfers.hpp"

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

std::unordered_map<const void*, tt_ldu_meta> ldu_tt_meta_map;

tt_ldu_meta& ensure_lduMat_on_device(KernelLauncher& k, const Foam::lduMatrix* lduMat, bool reserve_all_parts) {

    auto& tt_meta = get_tt_meta(lduMat, ldu_tt_meta_map);

    if (!tt_meta.addrs_on_device_) {
        #ifdef TRACY_ENABLE
        ZoneScopedN("TT Meta ldu copy addrs");
        #endif
        copy_ldu_addrs_to_device(k, tt_meta, lduMat);
    } else {
        #ifdef TRACY_ENABLE
        ZoneScopedN("TT Meta ldu reuse addrs");
        #endif
    }

    if (!tt_meta.contents_on_device_ ||
        (tt_meta.contents_on_device_ && (!reserve_all_parts ^ (tt_meta.lower_contents_start_ == tt_meta.upper_contents_start_)))
    ) { // if not uploaded, or if uploaded in a different format than requested now (i.e. reserve_all_parts changed) [we should be good with unreserved, unless we are running an operation that makes it asymmetric, in which case it is implicitly all reserved]
        #ifdef TRACY_ENABLE
        ZoneScopedN("TT Meta ldu copy contents");
        #endif
        copy_ldu_contents_to_device(k, tt_meta, lduMat, reserve_all_parts);
    } else {
        #ifdef TRACY_ENABLE
        ZoneScopedN("TT Meta ldu reuse contents");
        #endif
    }

    return tt_meta;
}

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
        }
        if (clear_contents && meta.contents_on_device_) {
            #ifdef TRACY_ENABLE
            ZoneScopedN("TT Meta clear contents");
            #endif
            meta.contents_on_device_ = false;
        }
    } // never uploaded to begin with
}