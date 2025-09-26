
#include <cstdint>
#include <stdint.h>
#include "dataflow_api.h"

#include "debug/dprint.h"
#include "tt-metalium/math.hpp"

#ifndef KERNEL_OP
#define KERNEL_OP +
#endif

#define STRINGIFY(str) #str
#define KERNEL_OP_STR STRINGIFY(KERNEL_OP)

void kernel_main() {
    // same arg indices as in reader_binary_diff_lenghts for compat
    uint32_t lduDestContentsAddr = get_arg_val<uint32_t>(0);
    uint32_t lduDestCellCount = get_arg_val<uint32_t>(1);
    uint32_t lduDestLowerStart = get_arg_val<uint32_t>(2);
    uint32_t lduDestSparseCount = get_arg_val<uint32_t>(3);
    uint32_t lduDestUpperStart = get_arg_val<uint32_t>(4);

    uint32_t lduAContentsAddr = get_arg_val<uint32_t>(5);
    uint32_t lduACellCount = get_arg_val<uint32_t>(6);
    uint32_t lduALowerStart = get_arg_val<uint32_t>(7);
    uint32_t lduASparseCount = get_arg_val<uint32_t>(8);
    uint32_t lduAUpperStart = get_arg_val<uint32_t>(9);

    uint32_t mode_mask = get_arg_val<uint32_t>(10);

    constexpr uint8_t matDest_cb = 1;
    constexpr uint8_t matA_cb = 2;

    const bool dest_in_symmetric = mode_mask & 0x1;
    const bool a_symmetric = mode_mask & 0x2;
    const bool dest_out_expand = mode_mask & 0x4;
    const bool a_diag_zero = mode_mask & 0x8;
    const bool dest_in_diag_zero = mode_mask & 0x10;
    const bool a_triang_zero = mode_mask & 0x20;

    const uint32_t page_size = 1024;

    const DataFormat data_format = get_dataformat(matDest_cb);

    const InterleavedAddrGenFast<true> lduDestDat_gen = {
        .bank_base_address = lduDestContentsAddr, .page_size = page_size, .data_format = data_format
    };

    const InterleavedAddrGenFast<true> lduADat_gen = {
        .bank_base_address = lduAContentsAddr, .page_size = page_size, .data_format = data_format
    };
    DPRINT << "TT Up " << "d" << lduDestCellCount << ", s" << lduDestSparseCount << ENDL();


    uint32_t page_count = (lduDestUpperStart+lduDestSparseCount + page_size/4 -1) / (page_size / 4);
    cb_reserve_back(matDest_cb, (page_count+3)/4);
    float* dest_diag_ptr = (float*) get_write_ptr(matDest_cb);
    float* dest_lower_ptr = dest_diag_ptr + lduDestLowerStart;
    float* dest_upper_ptr = dest_diag_ptr + lduDestUpperStart;
    for (uint32_t i = 0; i < page_count; ++i) {
        noc_async_read_tile(i, lduDestDat_gen, get_write_ptr(matDest_cb) + page_size * i);
    }

    page_count = (lduAUpperStart+lduASparseCount + page_size/4 -1) / (page_size / 4);
    cb_reserve_back(matA_cb, (page_count+3)/4);
    float* a_diag_ptr = (float*) get_write_ptr(matA_cb);
    float* a_lower_ptr = a_diag_ptr + lduALowerStart;
    float* a_upper_ptr = a_diag_ptr + lduAUpperStart;
    for (uint32_t i = 0; i < page_count; ++i) {
        noc_async_read_tile(i, lduADat_gen, get_write_ptr(matA_cb) + page_size * i);
    }

    noc_async_read_barrier();
    {
        DeviceZoneScopedN("matOpAssign ready");
    }
    DPRINT << "All noc reads done" << ENDL();

    if (!a_diag_zero) {
        for (uint32_t i = 0; i < lduDestCellCount; ++i) {
            auto aDiag = a_diag_ptr[i];
            auto orgDiag = dest_in_diag_zero? 0.0f : dest_diag_ptr[i];
            auto res = orgDiag KERNEL_OP aDiag;
            dest_diag_ptr[i] = res;
            DPRINT << "diag " << i << " " << orgDiag << " " << KERNEL_OP_STR << " " << aDiag << " -> " << res << ENDL();
        }
    }

    {
        DeviceZoneScopedN("matOpAssign diag done");
    }

    if (!a_triang_zero) {
        for (uint32_t i = 0; i < lduDestSparseCount; ++i) {
            auto a_l_val = a_lower_ptr[i];
            auto org_l_val = dest_lower_ptr[i];
            auto dest_l_val = org_l_val KERNEL_OP a_l_val;
            dest_lower_ptr[i] = dest_l_val;
            DPRINT << "spar " << i << ENDL();
            DPRINT << " lower/symm " << org_l_val << " " << KERNEL_OP_STR << " " << a_l_val << " -> " << dest_l_val << ENDL();
            if (!dest_in_symmetric || dest_out_expand) {
                auto org_u_val = dest_out_expand? org_l_val : dest_upper_ptr[i];
                auto a_u_val = a_symmetric ? a_l_val : a_upper_ptr[i];
                auto dest_u_val = org_u_val KERNEL_OP a_u_val;
                dest_upper_ptr[i] = dest_u_val;
                DPRINT << " upper " << org_u_val << " " << KERNEL_OP_STR << " " << a_u_val << " -> " << dest_u_val << ENDL();
            }
        }
    }
    {
        DeviceZoneScopedN("matOpAssign sparse done");
    }

    page_count = (lduDestUpperStart+lduDestSparseCount + page_size/4 -1) / (page_size / 4); // relies on dest being pre-allocated with the needed space (uppr and lower separate if expand is set)
    for (uint32_t i = 0; i < page_count; ++i) {
        noc_async_write_tile(i, lduDestDat_gen, get_write_ptr(matDest_cb) + page_size * i);
    }

    {
        DeviceZoneScopedN("matOpAssign write done");
    }

    noc_async_write_barrier();
}


