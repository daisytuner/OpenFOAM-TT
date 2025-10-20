// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <algorithm>
#include <compute_kernel_api/common.h>
#include "compute_kernel_api/eltwise_binary.h"
#include "compute_kernel_api/tile_move_copy.h"
#include <unistd.h>
#include <tools/profiler/kernel_profiler.hpp>

#ifndef FACE_LAYOUT
#define FACE_LAYOUT 1
#endif

namespace NAMESPACE {

/**
 * Custom for 1-tile width
 */
uint32_t faced_offset(uint32_t row, uint32_t col) {
    uint32_t in_face_row = row & 0xF;
    uint32_t face_id = ((row & 0x10) >> 3) | ((col & 0x10) >> 4);
    face_id += (row >> 5) * 4;
    uint32_t in_face_col = col & 0xF;

    return face_id * (16 * 16) + in_face_row * 16 + in_face_col;
}

bool collect_for(float* result, uint32_t addr, float* vec_ptr, uint32_t vec_chunk_offset, uint32_t vec_chunk_offset_end, uint32_t rowIdx, uint32_t colIdx) {

    if (addr < vec_chunk_offset || addr >= vec_chunk_offset_end) {
        if (addr == UINT32_MAX) {
            return true;
        }
        DPRINT << " col [" << rowIdx << ", " << colIdx << "]: " << addr << " not in range" << ENDL();
    } else {
        auto val = vec_ptr[addr - vec_chunk_offset];
        *result = val;

        DPRINT << " col [" << rowIdx << ", " << colIdx << "] = " << val << ENDL();
    }

    return false;
}

void collect_for_mul_tile(uint32_t* addr_ptr, float* collect_ptr, float* vec_ptr, uint32_t vec_chunk_offset, uint32_t vecs_per_chunk) {
    const uint32_t vec_chunk_end = vec_chunk_offset + vecs_per_chunk;

    constexpr uint32_t NEXT_ROW_OFFSET = (FACE_LAYOUT == 1) ? 16 : 32;
    constexpr uint32_t NEXT_FACE_ROW_OFFSET = (FACE_LAYOUT == 1) ? (16*16 + 16) : 32;
    constexpr uint32_t NEXT_FACE_COL_OFFSET = (FACE_LAYOUT == 1) ? (16*16 - 15) : 1;


    float* collect_row = collect_ptr;
    for (int rowIdx = 0; rowIdx < 32; ++rowIdx) {
        uint32_t* addr_row = addr_ptr + rowIdx * 32;
        bool upper_face = rowIdx < 16;

        float* collect = collect_row;
        for (int colIdx = 0; colIdx < 32; ++colIdx) {

            uint32_t adr = addr_row[colIdx];

            bool line_done = collect_for(collect, adr, vec_ptr, vec_chunk_offset, vec_chunk_end, rowIdx, colIdx);
            if (line_done) {
                break;
            }

            if (colIdx == 15) {
                collect += NEXT_FACE_COL_OFFSET;
            } else {
                collect += 1;
            }
        }
        if (rowIdx == 15) {
            collect_row += NEXT_FACE_ROW_OFFSET;
        } else {
            collect_row += NEXT_ROW_OFFSET;
        }
    }
}

void compute_mat_mul(float* dat_ptr, uint32_t* addr_ptr, float* collect_ptr, float* res_ptr) {
    
    constexpr uint32_t NEXT_ROW_OFFSET = (FACE_LAYOUT == 1) ? 16 : 32;
    constexpr uint32_t NEXT_FACE_ROW_OFFSET = (FACE_LAYOUT == 1) ? (16*16 + 16) : 32;
    constexpr uint32_t NEXT_FACE_COL_OFFSET = (FACE_LAYOUT == 1) ? (16*16 - 15) : 1;

    float* collect_row = collect_ptr;
    float* dat_row = dat_ptr;
    for (int idx = 0; idx < 32; ++idx) { // row and col of result of matmul

        uint32_t* addr_row = addr_ptr + 32 * idx;

        float sum = 0.0f;
        float* dat_entry = dat_row;
        float* collect_entry = collect_row;
        for (int k = 0; k < 32; ++k) {
            bool face_left = k < 16;
            uint32_t adr = addr_row[k];
            if (adr != UINT32_MAX) {
                float mat_in = *dat_entry;
                float vec_in = *collect_entry;
                float elem_res = sum + mat_in * vec_in;

                // DPRINT << "  [" << idx << "," << k << "]: " << sum << " + "  << mat_in << " * " << vec_in << "  => " << elem_res << ENDL();

                sum = elem_res;
                if (k == 15) {
                    collect_entry += NEXT_FACE_COL_OFFSET;
                    dat_entry += NEXT_FACE_COL_OFFSET;
                } else {
                    collect_entry += 1;
                    dat_entry += 1;
                }
            } else {
                break; // early abort, because right now it's left-aligned
            }
        }
        res_ptr[idx] = sum;

        if (idx == 15) {
            collect_row += NEXT_FACE_ROW_OFFSET;
            dat_row += NEXT_FACE_ROW_OFFSET;
        } else {
            collect_row += NEXT_ROW_OFFSET;
            dat_row += NEXT_ROW_OFFSET;
        }
    }
}

void MAIN {
    uint32_t vec_chunks = get_common_arg_val<uint32_t>(0);
    // uint32_t cells = get_common_arg_val<uint32_t>(3);

    uint32_t batches = get_arg_val<uint32_t>(0);
    uint32_t tiles_per_batch = get_arg_val<uint32_t>(1);
    uint32_t num_tiles = get_arg_val<uint32_t>(2);


    constexpr uint8_t cb_res = 0;
    constexpr uint8_t cb_dat = 1;
    constexpr uint8_t cb_addr = 2;
    constexpr uint8_t cb_vec = 3;
    constexpr uint8_t cb_collect = 4;

    constexpr uint32_t vec_page_size = 1024;
    constexpr uint32_t vecs_per_page = vec_page_size / 4;
    constexpr uint32_t vecs_per_chunk = vecs_per_page;
    constexpr uint32_t vecs_per_mat_tile = 32;
    constexpr uint32_t tiles_per_result_page = vecs_per_chunk / 32;

    binary_op_init_common(cb_dat, cb_dat, cb_collect);  // Unpack, Math, Pack
    add_tiles_init(cb_dat, cb_dat);

    UNPACK(DPRINT << "ellpack matVec up: " << batches << " batch (" << tiles_per_batch << " tiles/batch), " << num_tiles << " tiles total, " << vec_chunks << " vec chunks" << ENDL());

    uint32_t tile = 0;
    for (uint32_t b = 0; b < batches; ++b) {
        DeviceZoneScopedN("Batch");
        uint32_t end_tile_in_batch = std::min(num_tiles, tile + tiles_per_batch);

        tile_regs_acquire();

        {
            DeviceZoneScopedN("WaitForCbTiles");
            cb_wait_front(cb_dat, tiles_per_batch);
            cb_wait_front(cb_addr, tiles_per_batch);
            cb_wait_front(cb_collect, tiles_per_batch);
        }

#ifdef TRISC_UNPACK
        {
            UNPACK(uint32_t* addr_ptr = reinterpret_cast<uint32_t*>(CB_RD_PTR(cb_addr)));
            UNPACK(float* collect_ptr = reinterpret_cast<float*>(CB_RD_PTR(cb_collect)));
            DeviceZoneScopedN("Collect");

            for (uint32_t v = 0; v < vec_chunks; ++v) {
                cb_wait_front(cb_vec, 1);
                UNPACK(float* vec_ptr = reinterpret_cast<float*>(CB_RD_PTR(cb_vec)));

                UNPACK(uint32_t* tile_addr_ptr = addr_ptr);
                UNPACK(float* tile_collect_ptr = collect_ptr);
                for (uint32_t i = tile; i < end_tile_in_batch; ++i) {
                    float* dat_ptr = reinterpret_cast<float*>(CB_RD_PTR(cb_dat));
                    UNPACK(DPRINT << "Collecting for tile " << i+1 << "/" << tiles_per_batch << ", v" << v << ENDL());
                    UNPACK(collect_for_mul_tile(tile_addr_ptr, tile_collect_ptr, vec_ptr, v * vecs_per_chunk, vecs_per_chunk));

                    UNPACK(tile_addr_ptr += 1024);
                    UNPACK(tile_collect_ptr += 1024);
                }
                cb_pop_front(cb_vec, 1);
            }
        }
#endif

        UNPACK(DPRINT << "Unpack done" << ENDL());

        float* collect_ptr, *dat_ptr;
        uint32_t* addr_ptr;
        {
            DeviceZoneScopedN("GetTileCollect");
            // UNPACK((llk_unpack_get_tile<false, true>(cb_collect, 0, (uint32_t*)&collect_ptr)));
            // PACK(llk_pack_get_tile(cb_collect, 0, (uint32_t*)&collect_ptr));
            cb_get_tile(cb_collect, 0, &collect_ptr);
        }

        {
            DeviceZoneScopedN("GetTileDat");
            // UNPACK((llk_unpack_get_tile<false, true>(cb_dat, 0, (uint32_t*)&dat_ptr)));
            // PACK(llk_pack_get_tile(cb_dat, 0, (uint32_t*)&dat_ptr));
            cb_get_tile(cb_dat, 0, &dat_ptr);
        }
        {
            DeviceZoneScopedN("GetTileAddr");
            // UNPACK((llk_unpack_get_tile<false, true>(cb_addr, 0, (uint32_t*)&addr_ptr)));
            // PACK(llk_pack_get_tile(cb_addr, 0, (uint32_t*)&addr_ptr));
            cb_get_tile(cb_addr, 0, &addr_ptr);
        }


        // Because for some magic and undocumented reason cb_get_tile does read_ptr-1, which is 1 L1 line before the actual data
        collect_ptr += 4;
        dat_ptr += 4;
        addr_ptr += 4;

        PACK(DPRINT << "Starting Pack batch" << ENDL());

        // now we have assembled mat-tiles in cb_collect matching each tile in cb_dat for mat_mul, where the first line

        cb_reserve_back(cb_res, 1);
#ifdef TRISC_PACK
        PACK(float* wr_ptr = reinterpret_cast<float*>(CB_WR_PTR(cb_res)));
        for (; tile < end_tile_in_batch; ++tile) {
            PACK(DPRINT << " MatMul tile " << tile+1 << "/" << tiles_per_batch << ENDL());

            PACK(compute_mat_mul(dat_ptr, addr_ptr, collect_ptr, wr_ptr));

            PACK(dat_ptr += 1024); // in float, not bytes
            PACK(addr_ptr += 1024);
            PACK(collect_ptr += 1024);
            PACK(wr_ptr += 32);
        }
#endif

        tile_regs_wait();

        tile_regs_commit();

        PACK(DPRINT << "Pushing result" << ENDL());
        cb_push_back(cb_res, 1);
        // UNPACK((llk_unpack_release_tile<false, true>(cb_collect)));
        // PACK(llk_pack_release_tile(cb_collect));
//        cb_release_tile(cb_collect);
        {
            DeviceZoneScopedN("ReleaseCollect");
            // UNPACK(DPRINT << "Releasing collect" << ENDL());
            cb_pop_front(cb_collect, tiles_per_batch);
        }

        // cb_release_tile(cb_dat);
        // UNPACK(DPRINT << "Releasing dat" << ENDL());
        cb_pop_front(cb_dat, tiles_per_batch);

        // cb_release_tile(cb_addr);
        // UNPACK((llk_unpack_release_tile<false, true>(cb_addr)));
        // PACK(llk_pack_release_tile(cb_addr));
        // UNPACK(DPRINT << "Releasing addr" << ENDL());
        cb_pop_front(cb_addr, tiles_per_batch);

        
        tile_regs_release();
    }

    // DPRINT << "Ellpack Compute done" << ENDL();
}

}  // namespace NAMESPACE
