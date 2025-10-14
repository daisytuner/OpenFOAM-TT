// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <compute_kernel_api/common.h>
#include <compute_kernel_api/eltwise_binary.h>

using std::uint32_t;

namespace NAMESPACE {

void collect_for_mul_tile(uint32_t* addr_ptr, float* collect_ptr, float* vec_ptr, uint32_t vec_chunk_offset) {
    for (int rowIdx = 0; rowIdx < 32; ++rowIdx) { // row and col of ellpack dat/addr. Transposed for collect
        uint32_t* addr_row = addr_ptr + rowIdx * 32;
        float* collect_col = collect_ptr + rowIdx;

        for (int colIdx = 0; colIdx < 32; ++colIdx) {
            float* collect = collect_col + colIdx * 32;

            uint32_t adr = addr_row[colIdx];

            if (adr < vec_chunk_offset || adr >= vec_chunk_offset + 32) {
                collect[0] = 0.0f;
                if (adr == UINT32_MAX) {
                    break;
                }
            } else {
                collect[0] = vec_ptr[adr - vec_chunk_offset];
            }
        }
    }
}

void compute_mat_mul(float* dat_ptr, uint32_t* addr_ptr, float* collect_ptr, float* res_ptr) {

    for (int idx = 0; idx < 32; ++idx) { // row and col of result of matmul
        float* collect_col = collect_ptr + idx;
        uint32_t* addr_row = addr_ptr + 32 * idx;
        float* dat_row = dat_ptr + 32 * idx;
        float sum = 0.0f;
        for (int k = 0; k < 32; ++k) {
            uint32_t adr = addr_row[k];
            if (adr != UINT32_MAX) {
                sum += dat_row[k] * collect_col[k * 32];
            } else {
                break; // early abort, because right now it's left-aligned
            }
        }
        res_ptr[idx] = sum;
    }
}

void MAIN {
    uint32_t vec_chunks = get_common_arg_val<uint32_t>(0);

    uint32_t num_tiles = get_arg_val<uint32_t>(0);
    uint32_t batch_tiles = get_arg_val<uint32_t>(1);


    constexpr uint8_t cb_res = 0;
    constexpr uint8_t cb_dat = 1;
    constexpr uint8_t cb_addr = 2;
    constexpr uint8_t cb_vec = 3;
    constexpr uint8_t cb_collect = 4;

    for (uint32_t i = 0; i < num_tiles; i += batch_tiles) {

        cb_wait_front(cb_dat, batch_tiles);
        cb_wait_front(cb_addr, batch_tiles);
        cb_wait_front(cb_collect, batch_tiles);

        for (uint32_t v = 0; v < vec_chunks; ++v) {
            cb_wait_front(cb_vec, 1);
            UNPACK(float* vec_ptr = reinterpret_cast<float*>(CB_RD_PTR(cb_vec)));
            UNPACK(DPRINT << "Fetching vec chunk " << v << ENDL());

            UNPACK(uint32_t* addr_ptr = reinterpret_cast<uint32_t*>(CB_RD_PTR(cb_addr)));
            UNPACK(float* collect_ptr = reinterpret_cast<float*>(CB_RD_PTR(cb_collect)));
            for (uint32_t b = 0; b < batch_tiles; ++b) {
                UNPACK(collect_for_mul_tile(addr_ptr, collect_ptr, vec_ptr, v * 32));

                UNPACK(addr_ptr += 1024);
                UNPACK(collect_ptr += 1024);
                UNPACK(DPRINT << "Processed batch " << b+1 << "/" << batch_tiles << ENDL());

                for (int k = 0; k < 32; ++k) {
                    UNPACK(DPRINT << "[" << k << "]=" << collect_ptr[k] << ENDL());
                }
            }
            cb_pop_front(cb_vec, 1);
        }

        UNPACK(DPRINT << "Unpack done" << ENDL());

        float* collect_ptr;
        cb_get_tile(cb_collect, 0, collect_ptr);

        PACK(DPRINT << "Starting Pack batch" << ENDL());

        // now we have assembled mat-tiles in cb_collect matching each tile in cb_dat for mat_mul, where the first line

        PACK(float* dat_ptr = reinterpret_cast<float*>(CB_RD_PTR(cb_dat)));
        PACK(uint32_t* addr_ptr = reinterpret_cast<uint32_t*>(CB_RD_PTR(cb_addr)));
        PACK(float* wr_ptr = reinterpret_cast<float*>(CB_WR_PTR(cb_res)));
        for (uint32_t b = 0; b < batch_tiles; ++b) {
            cb_reserve_back(cb_res, 1);

            PACK(compute_mat_mul(dat_ptr, addr_ptr, collect_ptr, wr_ptr));

            for (int k = 0; k < 32; ++k) {
                PACK(DPRINT << "[" << k << "]=" << wr_ptr[k] << ENDL());
            }

            cb_push_back(cb_res, 1);

            PACK(dat_ptr += 1024);
            PACK(addr_ptr += 1024);
            PACK(collect_ptr += 1024);
            PACK(wr_ptr += 32);
        }

        cb_release_tile(cb_collect);

        UNPACK(DPRINT << "Releasing inputs" << ENDL());
        cb_pop_front(cb_dat, batch_tiles);
        cb_pop_front(cb_addr, batch_tiles);
        cb_pop_front(cb_collect, batch_tiles);

        tile_regs_acquire();
        tile_regs_commit();
        tile_regs_wait();
        tile_regs_release();
    }
}

}  // namespace NAMESPACE
