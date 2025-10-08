
#include <cstdint>
#include "dataflow_api.h"

#include "debug/dprint.h"
#include "tt-metalium/math.hpp"


void kernel_main() {
    uint32_t a_base_addr = get_common_arg_val<uint32_t>(0);
    uint32_t b_base_addr = get_common_arg_val<uint32_t>(1);

    uint32_t first_tile_offset = get_arg_val<uint32_t>(0);
    uint32_t num_tiles = get_arg_val<uint32_t>(1);

    constexpr uint8_t cb_a = 1;
    constexpr uint8_t cb_b = 2;

    constexpr uint32_t page_size = get_tile_size(cb_a);
    // assert(page_size == get_tile_size(cb_b));

    constexpr auto a_args = TensorAccessorArgs<0, 2>();
    const auto a_buf = TensorAccessor(a_args, a_base_addr, page_size);

    constexpr auto b_args = TensorAccessorArgs<a_args.next_compile_time_args_offset(), a_args.next_common_runtime_args_offset()>();
    const auto b_buf = TensorAccessor(b_args, b_base_addr, page_size);

    DPRINT << "Dense binOp Up " << ENDL();


    uint32_t end_tile = first_tile_offset + num_tiles;
    for (uint32_t tile = first_tile_offset; tile < end_tile; ++tile) {
        cb_reserve_back(cb_a, 1);
        cb_reserve_back(cb_b, 1);

        noc_async_read_page(tile, a_buf, get_write_ptr(cb_a));
        noc_async_read_page(tile, b_buf, get_write_ptr(cb_b));

        noc_async_read_barrier();

        // float* ptr_a = reinterpret_cast<float*>(get_write_ptr(cb_a));
        // float* ptr_b = reinterpret_cast<float*>(get_write_ptr(cb_b));

        // DPRINT << "a t" << tile << ":\n" << TileSlice(cb_a, 0, SliceRange::h0_w0_32(), TSLICE_OUTPUT_CB, TSLICE_WR_PTR, true, true) << ENDL();
        // DPRINT << "b t" << tile << ":\n" << TileSlice(cb_b, 0, SliceRange::h0_w0_32(), TSLICE_OUTPUT_CB, TSLICE_WR_PTR, true, true) << ENDL();

        cb_push_back(cb_a, 1);
        cb_push_back(cb_b, 1);
    }
}


