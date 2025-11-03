
#include <cstdint>
#include <compute_kernel_api/common.h>
#include <compute_kernel_api/cb_api.h>
#include <compute_kernel_api/tile_move_copy.h>
#include <compute_kernel_api/eltwise_unary/eltwise_unary.h>
#include <compute_kernel_api/eltwise_unary/negative.h>

namespace NAMESPACE {

void MAIN {
    
    uint32_t num_tiles = get_arg_val<uint32_t>(0);

    constexpr uint8_t cb_res = 0;
    constexpr uint8_t cb_a = 1;

    unary_op_init_common(cb_a, cb_res);

    copy_tile_init(cb_a);

    negative_tile_init();

    for (uint32_t i = 0; i < num_tiles; ++i) {

        tile_regs_acquire();

        cb_wait_front(cb_a, 1);
        copy_tile(cb_a, /*offset*/ 0, /*register_offset*/ 0);

        negative_tile(0);
        tile_regs_commit();
        tile_regs_wait();

        cb_reserve_back(cb_res, 1);
        pack_tile(0, cb_res);
        cb_pop_front(cb_a, 1);
        tile_regs_release();

        cb_push_back(cb_res, 1);
    }
}

}  // namespace NAMESPACE