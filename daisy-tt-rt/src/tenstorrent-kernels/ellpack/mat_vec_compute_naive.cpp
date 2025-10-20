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

using std::uint32_t;

namespace NAMESPACE {

void collect_for_mul_tile(uint32_t* addr_ptr, float* collect_ptr, float* vec_ptr, uint32_t vec_chunk_offset, uint32_t vecs_per_chunk) {
    const uint32_t vec_chunk_end = vec_chunk_offset + vecs_per_chunk;

    for (int rowIdx = 0; rowIdx < 32; ++rowIdx) { // row and col of ellpack dat/addr. Transposed for collect
        uint32_t* addr_row = addr_ptr + rowIdx * 32;
        float* collect_col = collect_ptr + rowIdx;

        for (int colIdx = 0; colIdx < 32; ++colIdx) {
            float* collect = collect_col + colIdx * 32;

            uint32_t adr = addr_row[colIdx];

            if (adr < vec_chunk_offset || adr >= vec_chunk_end) {
                if (adr == UINT32_MAX) {
                    break;
                }
                // DPRINT << " col [" << colIdx << ", " << rowIdx << "]: " << adr << " not in range" << ENDL();
            } else {
                auto val = vec_ptr[adr - vec_chunk_offset];
                *collect = val;
                
                // DPRINT << " col [" << colIdx << ", " << rowIdx << "] = " << val << ENDL();
                
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
                float mat_in = dat_row[k];
                float vec_in = collect_col[k * 32];
                float elem_res = sum + mat_in * vec_in;

                // DPRINT << "  [" << idx << "," << k << "]: " << sum << " + "  << mat_in << " * " << vec_in << "  => " << elem_res << ENDL();

                sum = elem_res;
            } else {
                break; // early abort, because right now it's left-aligned
            }
        }
        res_ptr[idx] = sum;
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
