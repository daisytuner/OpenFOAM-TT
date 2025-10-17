
#include <cstdint>
#include <compute_kernel_api/common.h>
#include <compute_kernel_api/eltwise_binary.h>

#include <debug/dprint.h>

#ifndef KERNEL_OP
#define KERNEL_OP 1
#endif

#if KERNEL_OP == 0
#define op_tiles add_tiles
#define op_tiles_init add_tiles_init
#elif KERNEL_OP == 1
#define op_tiles sub_tiles
#define op_tiles_init sub_tiles_init
#endif

// #define EVAL(x) x

// #define OP_TILES_INIT_FUNC(op) op ## _tiles_init
// #define OP_TILES_INIT_FUNC_W(op) OP_TILES_INIT_FUNC(op)
// #define op_tiles_init OP_TILES_INIT_FUNC(KERNEL_OP)

// #define OP_TILES_FUNC(op) op ## _tiles
// #define OP_TILES_FUNC_W(op) OP_TILES_FUNC(op)
// #define op_tiles OP_TILES_FUNC_W(KERNEL_OP)

namespace NAMESPACE {

void MAIN {
    
    uint32_t num_tiles = get_arg_val<uint32_t>(0);

    constexpr uint8_t cb_res = 0;
    constexpr uint8_t cb_a = 1;
    constexpr uint8_t cb_b = 2;

    binary_op_init_common(cb_a, cb_b, cb_res);

    op_tiles_init(cb_a, cb_b, false);
    
    for (uint32_t i = 0; i < num_tiles; ++i) {

        cb_wait_front(cb_a, 1);
        cb_wait_front(cb_b, 1);
        DPRINT_UNPACK(DPRINT << "start" << i << ENDL(); );

        // DPRINT_UNPACK({ DPRINT << " --READ--a-- " << TileSlice<128>(cb_a, 0, SliceRange::hw0_32_4()) << ENDL(); });

        // float* ptr_a = reinterpret_cast<float*>(CB_RD_PTR(cb_a));
        // float* ptr_b = reinterpret_cast<float*>(CB_RD_PTR(cb_b));

        // for (uint32_t x = 0; x < 32; ++x) {
        //     for (uint32_t y = 0; y < 32; ++y) {
        //         auto val_a = *ptr_a++;
        //         auto val_b = *ptr_b++;
        //         DPRINT_UNPACK(DPRINT << "[" << x << "][" << y << "]=" << val_a << " + " << val_b << ENDL());
        //     }
        // }

        tile_regs_acquire();
        DPRINT_MATH(DPRINT << "reg take" << i << ENDL(); );

        op_tiles(cb_a, cb_b, 0, 0, 0);

        tile_regs_commit();
        DPRINT_MATH(DPRINT << "reg done" << i << ENDL(); );

        cb_pop_front(cb_a, 1);
        cb_pop_front(cb_b, 1);
        DPRINT_UNPACK(DPRINT << "pop" << i << ENDL(); );

        DPRINT_PACK(DPRINT << "reserve" << i << ENDL(); );
        cb_reserve_back(cb_res, 1);

        tile_regs_wait();
        DPRINT_PACK(DPRINT << "reg take" << i << ENDL(); );
        pack_tile(0, cb_res);

        tile_regs_release();
        DPRINT_PACK(DPRINT << "reg free" << i << ENDL(); );

        // float* ptr_wr = reinterpret_cast<float*>(CB_WR_PTR(cb_res));

        // DPRINT_PACK(DPRINT << " --WRITE--res-- " << TileSlice<128>(cb_res, 0, SliceRange::hw0_32_4()) << ENDL(); );

        // for (uint32_t x = 0; x < 32; ++x) {
        //     for (uint32_t y = 0; y < 32; ++y) {
        //         auto val_a = *ptr_wr++;
        //         DPRINT_PACK(DPRINT << "[" << x << "][" << y << "]=" << val_a << ENDL());
        //     }
        // }

        cb_push_back(cb_res, 1);

        DPRINT_PACK(DPRINT << "push" << i << ENDL(); );
    }
}


}  // namespace NAMESPACE