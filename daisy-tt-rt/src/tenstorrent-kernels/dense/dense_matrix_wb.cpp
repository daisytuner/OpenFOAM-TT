
#include <cstdint>
#include "dataflow_api.h"

#include "debug/dprint.h"
#include "tt-metalium/math.hpp"


void kernel_main() {
    uint32_t dest_base_addr = get_common_arg_val<uint32_t>(0);
    uint32_t first_tile_offset = get_arg_val<uint32_t>(0);
    uint32_t num_tiles = get_arg_val<uint32_t>(1);

    constexpr uint8_t cb_out = 0;

    constexpr uint32_t page_size = get_tile_size(cb_out);

    constexpr auto dest_args = TensorAccessorArgs<0, 1>();
    const auto dest = TensorAccessor(dest_args, dest_base_addr, page_size);

    //dest_args.next_compile_time_args_offset()

    DPRINT << "Dense Wb Up " << ENDL();


    uint32_t end_tile = first_tile_offset + num_tiles;
    for (uint32_t tile = first_tile_offset; tile < end_tile; ++tile) {
        cb_wait_front(cb_out, 1);

        noc_async_write_page(tile, dest, get_read_ptr(cb_out));

        // float* ptr_a = reinterpret_cast<float*>(get_write_ptr(cb_out));

        // for (uint32_t x = 0; x < 32; ++x) {
        //     for (uint32_t y = 0; y < 32; ++y) {
        //         auto val_a = *ptr_a;
        //         DPRINT << "[" << x << "][" << y << "]=" << val_a << ENDL();
        //     }
        // }

        noc_async_writes_flushed(); // all reads from SRAM are done now

        cb_pop_front(cb_out, 1);
    }

    noc_async_write_barrier();
}


