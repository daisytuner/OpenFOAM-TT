#pragma once

#include <tt-metalium/buffer.hpp>
#include <memory>

struct tt_ldu_meta {
    bool contents_on_device_ = false;
    bool addrs_on_device_ = false;

    // matrix contents. They may change at some point. But also will not be changed on-device
    std::shared_ptr<tt::tt_metal::Buffer> d_data_ = nullptr;

    uint32_t cell_count = 0; //TODO we use this for diag Count & size of the virtual matrix. But it can happen that the diag is not yet allocated and we could save work (but this is more special case that diag is virtual 0, but it still should exist)
    uint32_t lower_contents_start_ = 0; // if lower_contents_start_ == upper_contents_start_ than it is mirrored. It does not hurt to read them twice
    uint32_t sparse_count = 0;
    uint32_t upper_contents_start_ = 0;
    bool lower_contains_also_upper = false; // symmetric matrix. irrespective of which matrix was populated in ldu, lower is always the present half.
    // upper may be reserved or not (upper_contents_start is either lower_contents_start or its own address)
    bool diag_zero = false; // if true, diag data is unitialized in buffer, but should be treated as 0
    bool triang_zero = false; // if true, lower and upper data is uninitialized in buffer, but should be treated as 0

    // addrs for the sparse data and interfaces. If those change after setup, I am giving up
    std::shared_ptr<tt::tt_metal::Buffer> d_addrs_ = nullptr;

    uint32_t upper_addrs_start_ = 0;
    uint32_t iface_map_start_ = 0;

    // -------------- dense meta
    bool dense_on_device_ = false;

    std::shared_ptr<tt::tt_metal::Buffer> d_dense_ = nullptr;

};

template<typename result> result& get_tt_meta(const void* key, std::unordered_map<const void*, result>& map) {
    auto it = map.find(key);
    if (it == map.end()) {
        map[key] = result();
        return map[key];
    } else {
        return it->second;
    }
}

template<typename result> void clear_tt_meta(const void* key, std::unordered_map<const void*, result>& map) {
    auto it = map.find(key);
    if (it != map.end()) {
        map.erase(it);
    }
}


