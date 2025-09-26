
#include <cstdint>
#include <stdint.h>
#include "dataflow_api.h"

#include "debug/dprint.h"
#include "tt-metalium/math.hpp"

void kernel_main() {
    uint32_t lduContentsAddr = get_arg_val<uint32_t>(0);
    uint32_t lduCellCount = get_arg_val<uint32_t>(1);
    uint32_t lduLowerStart = get_arg_val<uint32_t>(2);
    uint32_t lduSparseCount = get_arg_val<uint32_t>(3);
    uint32_t lduUpperStart = get_arg_val<uint32_t>(4);

    // const uint32_t neg_mode = get_ct_arg<0>();

    constexpr uint8_t addr_cb = 0;
    constexpr uint8_t mat_cb = 1;

    uint32_t page_size = 1024;

    const DataFormat data_format = get_dataformat(addr_cb);

    const InterleavedAddrGenFast<true> lduDat_gen = {
        .bank_base_address = lduContentsAddr, .page_size = page_size, .data_format = data_format
    };

    DPRINT << "TT Up " << "d" << lduCellCount << ", s" << lduSparseCount << ENDL();


    // uint32_t page_count = (iface_map_start + page_size/4 + page_size/4) / (page_size / 4);
    // cb_reserve_back(addr_cb, (page_count+3)/4);
    // uint32_t* lower_addr_ptr = (uint32_t*) get_write_ptr(addr_cb);
    // uint32_t* upper_addr_ptr = lower_addr_ptr + lduAddrUpperStart;
    // uint32_t* iface_addr_ptr = lower_addr_ptr + iface_map_start;
    // for (uint32_t i = 0; i < page_count; ++i) {
    //     noc_async_read_tile(i, lduAddr_gen, get_write_ptr(addr_cb) + page_size * i);
    // }

    uint32_t page_count = (lduUpperStart+lduSparseCount + page_size/4 -1) / (page_size / 4);
    cb_reserve_back(mat_cb, (page_count+3)/4);
    float* diag_ptr = (float*) get_write_ptr(mat_cb);
    float* lower_ptr = diag_ptr + lduLowerStart;
    float* upper_ptr = diag_ptr + lduUpperStart;
    for (uint32_t i = 0; i < page_count; ++i) {
        noc_async_read_tile(i, lduDat_gen, get_write_ptr(mat_cb) + page_size * i);
    }

    noc_async_read_barrier();
    {
        DeviceZoneScopedN("negate ready");
    }
    DPRINT << "All noc reads done" << ENDL();

    for (uint32_t i = 0; i < lduCellCount; ++i) {
        auto diag = -diag_ptr[i];
        diag_ptr[i] = diag;
        DPRINT << "diag " << i << " " << diag << ENDL();
    }

    for (uint32_t i = 0; i < lduSparseCount; ++i) {
        float l_val = lower_ptr[i];
        float res =  - l_val;
        lower_ptr[i] = res;
        DPRINT << "  " << l_val << " -> " << res << ENDL();
        
        if (lduUpperStart != lduLowerStart) {
            
            float u_val = upper_ptr[i];
            res = - u_val;
            upper_ptr[i] = res;
            DPRINT << "  " << u_val << " -> " << res << ENDL();
        }
    }
    {
        DeviceZoneScopedN("negate compute done");
    }

    for (uint32_t i = 0; i < page_count; ++i) {
        noc_async_write_tile(i, lduDat_gen, get_write_ptr(mat_cb) + page_size * i);
    }

    {
        DeviceZoneScopedN("negate write done");
    }

    noc_async_write_barrier();
}


