
#include "ttLduData.hpp"

#include "device_transfers.hpp"

std::unordered_map<const void*, tt_ldu_meta> ldu_tt_meta_map;

tt_ldu_meta& ensure_lduMat_on_device(KernelLauncher& k, const Foam::lduMatrix* lduMat) {

    auto& tt_meta = get_tt_meta(lduMat, ldu_tt_meta_map);

    if (!tt_meta.addrs_on_device_) {
        copy_ldu_addrs_to_device(k, tt_meta, lduMat);
    }

    if (!tt_meta.contents_on_device_) {
        copy_ldu_contents_to_device(k, tt_meta, lduMat);
    }

    return tt_meta;
}