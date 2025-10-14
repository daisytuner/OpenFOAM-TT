
#include <cstdint>
#include "dataflow_api.h"

#include "debug/dprint.h"
#include "tt-metalium/math.hpp"


void kernel_main() {
    uint32_t ell_val_base_addr = get_common_arg_val<uint32_t>(0);
    uint32_t ell_addr_base_addr = get_common_arg_val<uint32_t>(1);
    uint32_t inVec_base_addr = get_common_arg_val<uint32_t>(2);
    uint32_t vec_chunks = get_common_arg_val<uint32_t>(3);

    uint32_t first_tile_offset = get_arg_val<uint32_t>(0);
    uint32_t num_tiles = get_arg_val<uint32_t>(1); // how many tiles to read from the matrix
    uint32_t batch_tiles = get_arg_val<uint32_t>(2); // how many tiles will be cached, collecting vector parts

    constexpr uint8_t cb_dat = 1;
    constexpr uint8_t cb_addr = 2;
    constexpr uint8_t cb_inVec = 3;
    constexpr uint8_t cb_collect = 4;

    constexpr uint32_t mat_page_size = get_tile_size(cb_dat);
    constexpr uint32_t vec_page_size = get_tile_size(cb_inVec);

    constexpr auto ell_val_args = TensorAccessorArgs<0, 4>();
    const auto ell_val_buf = TensorAccessor(ell_val_args, ell_val_base_addr, mat_page_size);

    constexpr auto ell_addr_args = TensorAccessorArgs<ell_val_args.next_compile_time_args_offset(), ell_val_args.next_common_runtime_args_offset()>();
    const auto ell_addr_buf = TensorAccessor(ell_addr_args, ell_addr_base_addr, mat_page_size);

    constexpr auto vec_args = TensorAccessorArgs<ell_addr_args.next_compile_time_args_offset(), ell_addr_args.next_common_runtime_args_offset()>();
    const auto vec_buf = TensorAccessor(vec_args, inVec_base_addr, vec_page_size);

    DPRINT << "Ellpack matVec Rd ( " << first_tile_offset << ".+" << num_tiles << " in batches of " << batch_tiles << "), " << vec_chunks << " vec chunks" << ENDL();


    uint32_t end_tile = first_tile_offset + num_tiles;
    for (uint32_t tile = first_tile_offset; tile < end_tile;) {
        cb_reserve_back(cb_dat, batch_tiles);
        cb_reserve_back(cb_addr, batch_tiles);


        auto val_wr_addr = get_write_ptr(cb_dat);
        auto addr_wr_addr = get_write_ptr(cb_addr);
        for (uint32_t i = 0; i < batch_tiles; ++i, ++tile) {
            noc_async_read_page(tile, ell_val_buf, val_wr_addr);
            val_wr_addr += mat_page_size;
            noc_async_read_page(tile, ell_addr_buf, addr_wr_addr);
            addr_wr_addr += mat_page_size;
        }

        for (uint32_t vec_chunk = 0; vec_chunk < vec_chunks; ++vec_chunk) {
            cb_reserve_back(cb_inVec, 1);
            noc_async_read_page(tile, vec_buf, get_write_ptr(cb_inVec));
            noc_async_read_barrier();
            if (vec_chunk == 0) { // we let the batch reads run concurrent with the first vecChunk, so mark them as available now
                DPRINT << "a t" << tile << ":\n" << TileSlice(cb_dat, 0, SliceRange::h0_w0_32(), TSLICE_OUTPUT_CB, TSLICE_WR_PTR, true, true) << ENDL();
                DPRINT << "b t" << tile << ":\n" << TileSlice(cb_addr, 0, SliceRange::h0_w0_32(), TSLICE_OUTPUT_CB, TSLICE_WR_PTR, true, true) << ENDL();
                cb_push_back(cb_dat, batch_tiles);
                cb_push_back(cb_addr, batch_tiles);
            }
            cb_push_back(cb_inVec, 1);
            cb_reserve_back(cb_collect, batch_tiles);
            cb_push_back(cb_collect, batch_tiles);
        }



        // float* ptr_a = reinterpret_cast<float*>(get_write_ptr(cb_a));
        // float* ptr_b = reinterpret_cast<float*>(get_write_ptr(cb_b));


        tile += batch_tiles;
    }
}


