
#include <cstdint>

#include "dataflow_api.h"

#include "debug/dprint.h"

void kernel_main() {

    uint32_t dst_addr = get_common_arg_val<uint32_t>(0);

    uint32_t first_tile_offset = get_arg_val<uint32_t>(0);
    uint32_t num_tiles = get_arg_val<uint32_t>(1);

    constexpr uint32_t cb_out = 0;

    constexpr uint32_t page_size = get_tile_size(cb_out);

    constexpr auto dest_args = TensorAccessorArgs<0, 1>();
    const auto dest = TensorAccessor(dest_args, dst_addr, page_size);

    DPRINT << "Ellpack Wb up" << ENDL();

    uint32_t end_tile = first_tile_offset + num_tiles;
    for (uint32_t tile = first_tile_offset; tile < end_tile; ++tile) {
        cb_wait_front(cb_out, 1);

        DPRINT << "Wb got chunk " << tile << ENDL();

        noc_async_write_page(tile, dest, get_read_ptr(cb_out));

         float* ptr_a = reinterpret_cast<float*>(get_read_ptr(cb_out));

         for (uint32_t x = 0; x < 32; ++x) {
             auto val_a = *(ptr_a+x);
             DPRINT << "[" << x << "]=" << val_a << ENDL();
         }

        noc_async_writes_flushed(); // all reads from SRAM are done now

        cb_pop_front(cb_out, 1);

        DPRINT << "Wb done chunk " << tile << ENDL();
    }

    noc_async_write_barrier();
}
