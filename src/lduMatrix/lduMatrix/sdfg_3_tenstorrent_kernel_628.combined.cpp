#include <cstdint>
#include <dataflow_api.h>
#include <debug/dprint.h>
#include <tools/profiler/kernel_profiler.hpp>

#define __daisy_min(a, b) ((a) < (b) ? (a) : (b))
#define __daisy_max(a, b) ((a) > (b) ? (a) : (b))
#define __daisy_fma(a, b, c) a * b + c

void kernel_main() {
    uint32_t tt_first_unit = get_arg_val<uint32_t>(0);
    uint32_t tt_work_units = get_arg_val<uint32_t>(1);
    int _1 = get_arg_val<int >(2);
    float _2 = get_arg_val<float >(3);
    uint32_t tt_tensor_addr_0 = get_arg_val<uint32_t>(4); // base addr of __daisy_tt__3
    uint32_t tt_tensor_addr_1 = get_arg_val<uint32_t>(5); // base addr of __daisy_tt__5
    uint32_t tt_tensor_addr_2 = get_arg_val<uint32_t>(6); // base addr of __daisy_tt__6

    constexpr auto tt_tensor_args_0 = TensorAccessorArgs<0, 0>();
    const auto tt_tensor_0 = TensorAccessor(tt_tensor_args_0, tt_tensor_addr_0, 1024*4);
    constexpr auto tt_tensor_args_1 = TensorAccessorArgs<tt_tensor_args_0.next_compile_time_args_offset(), tt_tensor_args_0.next_common_runtime_args_offset()>();
    const auto tt_tensor_1 = TensorAccessor(tt_tensor_args_1, tt_tensor_addr_1, 1024*4);
    constexpr auto tt_tensor_args_2 = TensorAccessorArgs<tt_tensor_args_1.next_compile_time_args_offset(), tt_tensor_args_1.next_common_runtime_args_offset()>();
    const auto tt_tensor_2 = TensorAccessor(tt_tensor_args_2, tt_tensor_addr_2, 1024*4);

    long long _32_tile0 __attribute__((aligned(8)));
    long long _32 __attribute__((aligned(8)));

    DPRINT << "Up sdfg_3_tenstorrent_kernel_628.combined" << ENDL();

    cb_reserve_back(0, 1);
    cb_reserve_back(1, 1);
    cb_reserve_back(2, 1);
    uint32_t tt_last_unit = tt_first_unit + tt_work_units;
    _32_tile0 = tt_first_unit*1024;

    for(uint32_t tt_tile_idx = tt_first_unit; tt_tile_idx < tt_last_unit; ++tt_tile_idx) {

        uint32_t l1_write_addr_0 = get_write_ptr(0);
        uint32_t l1_write_addr_1 = get_write_ptr(1);
        uint32_t l1_write_addr_2 = get_write_ptr(2);

        {
            DeviceZoneScopedN("fetch")

            noc_async_read_page(tt_tile_idx, tt_tensor_0, l1_write_addr_0);
            noc_async_read_page(tt_tile_idx, tt_tensor_1, l1_write_addr_1);
            noc_async_read_page(tt_tile_idx, tt_tensor_2, l1_write_addr_2);

            noc_async_read_barrier();
        }

        {
            DeviceZoneScopedN("compute")

            void* __daisy_tt__3 = reinterpret_cast<void*>(l1_write_addr_0);
            void* __daisy_tt__5 = reinterpret_cast<void*>(l1_write_addr_1);
            void* __daisy_tt__6 = reinterpret_cast<void*>(l1_write_addr_2);

            uint32_t tt_idx_within_block;

            // Map
            for(tt_idx_within_block = 0, _32 = _32_tile0; (_32 < _1 && _32 < 1024 + _32_tile0); tt_idx_within_block += 1, _32 = 1 + _32)
            {
                    {
                        float _in1 = _2;
                        float _in3 = (reinterpret_cast<float *>(__daisy_tt__5))[tt_idx_within_block];
                        float _in2 = (reinterpret_cast<float *>(__daisy_tt__3))[tt_idx_within_block];
                        float _out;

                        _out = _in1 * _in2 + _in3;

                        (reinterpret_cast<float *>(__daisy_tt__6))[tt_idx_within_block] = _out;
                    }
            }
        }
        {
            DeviceZoneScopedN("wb")

            uint32_t l1_read_addr_2 = get_write_ptr(2);
            noc_async_write_page(tt_tile_idx, tt_tensor_2, l1_read_addr_2);

            noc_async_writes_flushed();
        }
        _32_tile0 += 1024;
    }

    noc_async_write_barrier();
}

