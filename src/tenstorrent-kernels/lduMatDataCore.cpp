
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
    uint32_t inVecAddr = get_arg_val<uint32_t>(8);
    uint32_t resVecAddr = get_arg_val<uint32_t>(9);
    // uint32_t ifaceCoeffsAddr = get_arg_val<uint32_t>(10);
    // uint32_t iface_count = get_arg_val<uint32_t>(11);



    constexpr uint8_t addr_cb = 0;
    constexpr uint8_t mat_cb = 1;
    constexpr uint8_t inVec_cb = 2;
    constexpr uint8_t resVec_cb = 3;
    constexpr uint8_t iface_cb = 4;

    uint32_t page_size = 1024;

    const DataFormat data_format = get_dataformat(addr_cb);

    const InterleavedAddrGenFast<true> lduAddr_gen = {
        .bank_base_address = lduAddrAddr, .page_size = page_size, .data_format = data_format
    };

    const InterleavedAddrGenFast<true> lduDat_gen = {
        .bank_base_address = lduContentsAddr, .page_size = page_size, .data_format = data_format
    };

    const InterleavedAddrGenFast<true> inVec_gen = {
        .bank_base_address = inVecAddr, .page_size = page_size, .data_format = data_format
    };

    const InterleavedAddrGenFast<true> resVec_gen = {
        .bank_base_address = resVecAddr, .page_size = page_size, .data_format = data_format
    };

    DPRINT << "TT Up " << "d" << lduCellCount << ", s" << lduSparseCount << ENDL();

    // const InterleavedAddrGenFast<true> ifaceCoeffs_gen = {
    //     .bank_base_address = ifaceCoeffsAddr, .page_size = page_size, .data_format = data_format
    // };


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

    page_count = (lduCellCount + page_size/4 -1)/ (page_size / 4);
    cb_reserve_back(inVec_cb, (page_count+3)/4);
    float* inVec_ptr = (float*)get_write_ptr(inVec_cb);
    for (uint32_t i = 0; i < page_count; ++i) {
        noc_async_read_tile(i, inVec_gen, get_write_ptr(inVec_cb) + page_size * i);
    }

    // page_count = (lduCellCount * iface_count + page_size/4 -1) / (page_size / 4);
    // cb_reserve_back(iface_cb, (page_count+3)/4);
    // float* ifaceCoeffs = (float*)get_write_ptr(iface_cb);
    // uint32_t* iface_meta_ptr = (uint32_t*)get_write_ptr(iface_cb);
    // for (uint32_t i = 0; i < page_count; ++i) {
    //     noc_async_read_tile(i, ifaceCoeffs_gen, get_write_ptr(iface_cb) + page_size * i);
    // }

    page_count = (lduCellCount + page_size/4 - 1) / (page_size / 4);
    cb_reserve_back(resVec_cb, (page_count+3)/4);
    float* res_ptr = (float*)get_write_ptr(resVec_cb);

    noc_async_read_barrier();
    DPRINT << "All noc reads done" << ENDL();

    // send inVec to interfaces

    for (uint32_t i = 0; i < lduCellCount; ++i) {
        auto diag = diag_ptr[i];
        auto in = inVec_ptr[i];
        auto res = diag * in;
        res_ptr[i] = res;
        DPRINT << "diag " << i << " " << diag << "*" << in << "=" << res << ENDL();
    }

    for (uint32_t i = 0; i < lduSparseCount; ++i) {
        auto l_idx = lower_addr_ptr[i];
        auto u_idx = upper_addr_ptr[i];
        DPRINT << "spar " << i << " " << l_idx << "," << u_idx << ENDL();
        float l_val = lower_ptr[i];
        float lower_val = l_val * inVec_ptr[l_idx];
        float res = res_ptr[u_idx] + lower_val;
        res_ptr[u_idx] = res;
        DPRINT << "  " << l_val << " -> " << res << ENDL();
        float u_val = upper_ptr[i];
        float upper_val = u_val * inVec_ptr[u_idx];
        res = res_ptr[l_idx] + upper_val;
        res_ptr[l_idx] = res;
        DPRINT << "  " << u_val << " -> " << res << ENDL();
    }

    // uint32_t incoming_idx = 0;
    // for (uint32_t i = 0; i < iface_count; ++i) {
    //     auto iface_idx = iface_meta_ptr[incoming_idx++];
    //     auto iface_meta_offset = iface_addr_ptr[iface_idx];
    //     auto iface_elem_count = iface_addr_ptr[iface_meta_offset++];
        
    //     DPRINT << "iface " << i << " start " << iface_idx << ", " << iface_meta_offset << ", " << iface_elem_count << " elems" << ENDL();
    //     for (int j = 0; j < iface_elem_count; ++j) {
    //         incoming_idx++;
    //         auto coeff = ifaceCoeffs[incoming_idx++];
    //         auto cell_idx = iface_addr_ptr[iface_meta_offset++];
    //         float foreign = 0.0f;
    //         res_ptr[cell_idx] -= coeff * foreign;
    //         DPRINT << "iface " << cell_idx << " -= " << coeff << "*" << foreign << ENDL();
    //     }
    // }

    for (uint32_t i = 0; i < page_count; ++i) {
        noc_async_write_tile(i, resVec_gen, get_write_ptr(resVec_cb) + page_size * i);
        DPRINT << "Res: " << get_write_ptr(resVec_cb) + page_size * i << ": " << res_ptr[i*page_size/4] << ENDL();
    }

    noc_async_write_barrier();
}


