
#include <cstdint>
#include <stdint.h>
#include "dataflow_api.h"

#include "debug/dprint.h"
#include "tt-metalium/math.hpp"

void kernel_main() {
    // same arg indices as in reader_binary_diff_lenghts for compat
    uint32_t lduAddrAddr = get_arg_val<uint32_t>(0);
    uint32_t lduAddrUpperStart = get_arg_val<uint32_t>(1);
    uint32_t iface_map_start = get_arg_val<uint32_t>(2);
    uint32_t lduContentsAddr = get_arg_val<uint32_t>(3);
    uint32_t lduCellCount = get_arg_val<uint32_t>(4);
    uint32_t lduLowerStart = get_arg_val<uint32_t>(5);
    uint32_t lduSparseCount = get_arg_val<uint32_t>(6);
    uint32_t lduUpperStart = get_arg_val<uint32_t>(7);

    const uint32_t neg_mode = get_ct_arg<0>();

    constexpr uint8_t addr_cb = 0;
    constexpr uint8_t mat_cb = 1;

    uint32_t page_size = 1024;

    const DataFormat data_format = get_dataformat(addr_cb);

    const InterleavedAddrGenFast<true> lduAddr_gen = {
        .bank_base_address = lduAddrAddr, .page_size = page_size, .data_format = data_format
    };

    const InterleavedAddrGenFast<true> lduDat_gen = {
        .bank_base_address = lduContentsAddr, .page_size = page_size, .data_format = data_format
    };

    DPRINT << "TT Up " << "d" << lduCellCount << ", s" << lduSparseCount << ENDL();


    uint32_t page_count = (iface_map_start + page_size/4 + page_size/4) / (page_size / 4);
    cb_reserve_back(addr_cb, (page_count+3)/4);
    uint32_t* lower_addr_ptr = (uint32_t*) get_write_ptr(addr_cb);
    uint32_t* upper_addr_ptr = lower_addr_ptr + lduAddrUpperStart;
    uint32_t* iface_addr_ptr = lower_addr_ptr + iface_map_start;
    for (uint32_t i = 0; i < page_count; ++i) {
        noc_async_read_tile(i, lduAddr_gen, get_write_ptr(addr_cb) + page_size * i);
    }

    page_count = (lduUpperStart+lduSparseCount + page_size/4 -1) / (page_size / 4);
    cb_reserve_back(mat_cb, (page_count+3)/4);
    float* diag_ptr = (float*) get_write_ptr(mat_cb);
    float* lower_ptr = diag_ptr + lduLowerStart;
    float* upper_ptr = diag_ptr + lduUpperStart;
    for (uint32_t i = 0; i < page_count; ++i) {
        noc_async_read_tile(i, lduDat_gen, get_write_ptr(mat_cb) + page_size * i);
    }

    noc_async_read_barrier();
    {
        DeviceZoneScopedN("sumDiag ready");
    }
    DPRINT << "All noc reads done" << ENDL();

    for (uint32_t i = 0; i < lduSparseCount; ++i) {
        auto l_idx = lower_addr_ptr[i];
        auto u_idx = upper_addr_ptr[i];
        DPRINT << "spar " << i << " " << l_idx << "," << u_idx << ENDL();
        float l_val = lower_ptr[i];
        float res = neg_mode? diag_ptr[l_idx] - l_val : diag_ptr[l_idx] + l_val;
        diag_ptr[l_idx] = res;
        DPRINT << "  " << l_val << " -> " << res << ENDL();
        float u_val = upper_ptr[i];
        res = neg_mode? diag_ptr[u_idx] - u_val : diag_ptr[u_idx] + u_val;
        diag_ptr[u_idx] = res;
        DPRINT << "  " << u_val << " -> " << res << ENDL();
    }
    {
        DeviceZoneScopedN("sumDiag compute done");
    }

    page_count = (lduUpperStart) / (page_size / 4);
    for (uint32_t i = 0; i < page_count; ++i) {
        noc_async_write_tile(i, lduDat_gen, get_write_ptr(mat_cb) + page_size * i);
    }

    {
        DeviceZoneScopedN("sumDiag write done");
    }

    noc_async_write_barrier();
}


