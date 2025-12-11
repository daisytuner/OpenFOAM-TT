#include "sdfg_3.h"

#include <cstdint>
#include <cstring>
#include <cassert>
#include <memory>
#include <vector>
#include <tt-metalium/host_api.hpp>
#include <tt-metalium/work_split.hpp>
#include <tt-metalium/tensor_accessor_args.hpp>
#include <daisy_rtl/global_tenstorrent_init.h>
#include <tracy/Tracy.hpp>
#include <tt-metalium/tt_metal_profiler.hpp>


static void __daisy_tt_h2d_transfer(
    tt::tt_metal::IDevice* device,
    int cq_no,
    std::shared_ptr<tt::tt_metal::Buffer> buffer,
    const void* src_ptr,
    size_t size,
    bool blocking
) {
    ZoneScopedN("daisy_tt_h2d_transfer");
    auto page_size = buffer->page_size();

    auto safe_read_bytes = tt::round_down(size, page_size);
    auto left_bytes = size - safe_read_bytes;

    if (safe_read_bytes > 0) {
        tt::tt_metal::EnqueueWriteSubBuffer(
            device->command_queue(cq_no),
            buffer,
            src_ptr,
            {0, safe_read_bytes},
            false
        );
    }

    if (safe_read_bytes < size) {

        uint8_t* temp = new uint8_t[page_size]{};

        const uint8_t* byte_src = reinterpret_cast<const uint8_t*>(src_ptr);

        memcpy(temp, byte_src+safe_read_bytes, left_bytes);

        tt::tt_metal::EnqueueWriteSubBuffer(
            device->command_queue(cq_no),
            buffer,
            temp,
            {safe_read_bytes, page_size},
            true
        );

        delete[] temp;
    }
}



static void __daisy_tt_d2h_transfer(
    tt::tt_metal::IDevice* device,
    int cq_no,
    std::shared_ptr<tt::tt_metal::Buffer> buffer,
    void* dst_ptr,
    size_t size,
    bool blocking
) {
    ZoneScopedN("daisy_tt_d2h_transfer");
    auto page_size = buffer->page_size();

    auto safe_write_bytes = tt::round_down(size, page_size);
    auto left_bytes = size - safe_write_bytes;

    if (safe_write_bytes > 0) {
        tt::tt_metal::EnqueueReadSubBuffer(
            device->command_queue(cq_no),
            buffer,
            dst_ptr,
            {0, safe_write_bytes},
            left_bytes == 0
        );
    }

    if (safe_write_bytes < size) {

        uint8_t* temp = new uint8_t[page_size];

        tt::tt_metal::EnqueueReadSubBuffer(
            device->command_queue(cq_no),
            buffer,
            temp,
            {safe_write_bytes, page_size},
            true
        );

        uint8_t* byte_dst = reinterpret_cast<uint8_t*>(dst_ptr);

        memcpy(byte_dst+safe_write_bytes, temp, left_bytes);

        delete[] temp;
    }
}


extern "C" void sdfg_3_func(void* _0, int _1, float _2, void* _3, float _4, void* _5, void* _6)
{
tt::tt_metal::IDevice* tt_device = daisy::tenstorrent::daisy_get_tt_device(0);

std::shared_ptr<tt::tt_metal::Buffer> __daisy_tt__6;
std::shared_ptr<tt::tt_metal::Buffer> __daisy_tt__3;
bool _8 __attribute__((aligned(1)));
float _50 __attribute__((aligned(4)));
long long _43_tile0 __attribute__((aligned(8)));
long long _32 __attribute__((aligned(8)));
long long _32_tile0 __attribute__((aligned(8)));
std::shared_ptr<tt::tt_metal::Buffer> __daisy_tt__5;
long long _43 __attribute__((aligned(8)));
long long _13 __attribute__((aligned(8)));
long long _13_tile0 __attribute__((aligned(8)));
bool _24 __attribute__((aligned(1)));
    {
        float _in2 = 1.0f;
        float _in1 = _2;
        bool __out;

        __out = _in1 == _in2;

        _8 = __out;
    }
    if((_8 != false))
    {
            __daisy_tt__6 = tt::tt_metal::CreateBuffer({
                .device = tt_device, 
                .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_1) / (4096))),
                .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
                .buffer_type = tt::tt_metal::BufferType::DRAM
            });
            __daisy_tt_h2d_transfer(
            	tt_device, 
            	0, 
            	__daisy_tt__6, _6, 
            	4*_1,
            	1
            );
            __daisy_tt__5 = tt::tt_metal::CreateBuffer({
                .device = tt_device, 
                .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_1) / (4096))),
                .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
                .buffer_type = tt::tt_metal::BufferType::DRAM
            });
            __daisy_tt_h2d_transfer(
            	tt_device, 
            	0, 
            	__daisy_tt__5, _5, 
            	4*_1,
            	1
            );
            __daisy_tt__3 = tt::tt_metal::CreateBuffer({
                .device = tt_device, 
                .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_1) / (4096))),
                .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
                .buffer_type = tt::tt_metal::BufferType::DRAM
            });
            __daisy_tt_h2d_transfer(
            	tt_device, 
            	0, 
            	__daisy_tt__3, _3, 
            	4*_1,
            	1
            );
            tt::tt_metal::Program tt_program;
            {
                tt::tt_metal::CoreCoord tt_full_grid = tt_device->compute_with_storage_grid_size();
                auto [num_cores, all_cores, main_cores, remainder_cores, units_per_main, units_per_remainder] = tt::tt_metal::split_work_to_cores(tt_full_grid, ((1023 + _1) / (1024)));
                auto tt_cb_0_config = tt::tt_metal::CircularBufferConfig(8192, {{0, tt::DataFormat::Float32}})
                	.set_page_size(0, 4096);
                auto tt_cb_0 = tt::tt_metal::CreateCircularBuffer(tt_program, all_cores, tt_cb_0_config);
                auto tt_cb_1_config = tt::tt_metal::CircularBufferConfig(8192, {{1, tt::DataFormat::Float32}})
                	.set_page_size(1, 4096);
                auto tt_cb_1 = tt::tt_metal::CreateCircularBuffer(tt_program, all_cores, tt_cb_1_config);
                auto tt_cb_2_config = tt::tt_metal::CircularBufferConfig(8192, {{2, tt::DataFormat::Float32}})
                	.set_page_size(2, 4096);
                auto tt_cb_2 = tt::tt_metal::CreateCircularBuffer(tt_program, all_cores, tt_cb_2_config);
                std::vector<uint32_t> compile_args_k0, rt_common_args_k0;
                tt::tt_metal::TensorAccessorArgs(__daisy_tt__3).append_to(compile_args_k0, rt_common_args_k0);
                tt::tt_metal::TensorAccessorArgs(__daisy_tt__5).append_to(compile_args_k0, rt_common_args_k0);
                tt::tt_metal::TensorAccessorArgs(__daisy_tt__6).append_to(compile_args_k0, rt_common_args_k0);
                compile_args_k0.insert(compile_args_k0.end(), {});
                auto kernel_movRd_0_0 = tt::tt_metal::CreateKernel(tt_program, "/home/ramon/git/OpenFOAM-TT/src/lduMatrix/lduMatrix/sdfg_3_tenstorrent_kernel_599.combined.cpp", all_cores, tt::tt_metal::ReaderDataMovementConfig(compile_args_k0, {}));
                tt::tt_metal::SetCommonRuntimeArgs(tt_program, kernel_movRd_0_0, rt_common_args_k0);

                uint32_t units_done = 0;
                for (auto& tt_range : all_cores.ranges()) {
                    for (auto& core : tt_range) {
                        uint32_t units_on_core = 0;
                        if (main_cores.contains(core)) {
                        	units_on_core = units_per_main;
                        } else if (remainder_cores.contains(core)) {
                        	units_on_core = units_per_remainder;
                        } else {
                        	TT_ASSERT(false, "Core not in specified core ranges");
                        }
                        tt::tt_metal::SetRuntimeArgs(tt_program, kernel_movRd_0_0, core, {units_done, units_on_core, static_cast<uint32_t>(_1), reinterpret_cast<uint32_t&>(_4), __daisy_tt__3->address(), __daisy_tt__5->address(), __daisy_tt__6->address()});
                        units_done += units_on_core;
                    }
                }

                tt::tt_metal::EnqueueProgram(tt_device->command_queue(0), tt_program, 1);

                double tt_num_cores_used = static_cast<double>(num_cores);
                double tt_num_cores_available = static_cast<double>((tt_full_grid.x * tt_full_grid.y));
                double tt_cores_used_rel = tt_num_cores_used / tt_num_cores_available;
                double tt_work_units_per_core = static_cast<double>(units_done) / tt_num_cores_available;

            }
            __daisy_tt_d2h_transfer(
            	tt_device,	0,
            	__daisy_tt__6, _6, 
            	4*_1,
            	1
            );
    }
    else if((_8 == false))
    {
            {
                float _in2 = 1.0f;
                float _in1 = _4;
                bool __out;

                __out = _in1 == _in2;

                _24 = __out;
            }
            if((_24 != false))
            {
                    __daisy_tt__5 = tt::tt_metal::CreateBuffer({
                        .device = tt_device, 
                        .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_1) / (4096))),
                        .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
                        .buffer_type = tt::tt_metal::BufferType::DRAM
                    });
                    __daisy_tt_h2d_transfer(
                    	tt_device, 
                    	0, 
                    	__daisy_tt__5, _5, 
                    	4*_1,
                    	1
                    );
                    __daisy_tt__6 = tt::tt_metal::CreateBuffer({
                        .device = tt_device, 
                        .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_1) / (4096))),
                        .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
                        .buffer_type = tt::tt_metal::BufferType::DRAM
                    });
                    __daisy_tt_h2d_transfer(
                    	tt_device, 
                    	0, 
                    	__daisy_tt__6, _6, 
                    	4*_1,
                    	1
                    );
                    __daisy_tt__3 = tt::tt_metal::CreateBuffer({
                        .device = tt_device, 
                        .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_1) / (4096))),
                        .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
                        .buffer_type = tt::tt_metal::BufferType::DRAM
                    });
                    __daisy_tt_h2d_transfer(
                    	tt_device, 
                    	0, 
                    	__daisy_tt__3, _3, 
                    	4*_1,
                    	1
                    );
                    {
                        tt::tt_metal::Program tt_program;
                        tt::tt_metal::CoreCoord tt_full_grid = tt_device->compute_with_storage_grid_size();
                        auto [num_cores, all_cores, main_cores, remainder_cores, units_per_main, units_per_remainder] = tt::tt_metal::split_work_to_cores(tt_full_grid, ((1023 + _1) / (1024)));
                        auto tt_cb_0_config = tt::tt_metal::CircularBufferConfig(8192, {{0, tt::DataFormat::Float32}})
                        	.set_page_size(0, 4096);
                        auto tt_cb_0 = tt::tt_metal::CreateCircularBuffer(tt_program, all_cores, tt_cb_0_config);
                        auto tt_cb_1_config = tt::tt_metal::CircularBufferConfig(8192, {{1, tt::DataFormat::Float32}})
                        	.set_page_size(1, 4096);
                        auto tt_cb_1 = tt::tt_metal::CreateCircularBuffer(tt_program, all_cores, tt_cb_1_config);
                        auto tt_cb_2_config = tt::tt_metal::CircularBufferConfig(8192, {{2, tt::DataFormat::Float32}})
                        	.set_page_size(2, 4096);
                        auto tt_cb_2 = tt::tt_metal::CreateCircularBuffer(tt_program, all_cores, tt_cb_2_config);
                        std::vector<uint32_t> compile_args_k0, rt_common_args_k0;
                        tt::tt_metal::TensorAccessorArgs(__daisy_tt__3).append_to(compile_args_k0, rt_common_args_k0);
                        tt::tt_metal::TensorAccessorArgs(__daisy_tt__5).append_to(compile_args_k0, rt_common_args_k0);
                        tt::tt_metal::TensorAccessorArgs(__daisy_tt__6).append_to(compile_args_k0, rt_common_args_k0);
                        compile_args_k0.insert(compile_args_k0.end(), {});
                        auto kernel_movRd_0_0 = tt::tt_metal::CreateKernel(tt_program, "/home/ramon/git/OpenFOAM-TT/src/lduMatrix/lduMatrix/sdfg_3_tenstorrent_kernel_628.combined.cpp", all_cores, tt::tt_metal::ReaderDataMovementConfig(compile_args_k0, {}));
                        tt::tt_metal::SetCommonRuntimeArgs(tt_program, kernel_movRd_0_0, rt_common_args_k0);

                        uint32_t units_done = 0;
                        for (auto& tt_range : all_cores.ranges()) {
                            for (auto& core : tt_range) {
                                uint32_t units_on_core = 0;
                                if (main_cores.contains(core)) {
                                	units_on_core = units_per_main;
                                } else if (remainder_cores.contains(core)) {
                                	units_on_core = units_per_remainder;
                                } else {
                                	TT_ASSERT(false, "Core not in specified core ranges");
                                }
                                tt::tt_metal::SetRuntimeArgs(tt_program, kernel_movRd_0_0, core, {units_done, units_on_core, static_cast<uint32_t>(_1), reinterpret_cast<uint32_t&>(_2), __daisy_tt__3->address(), __daisy_tt__5->address(), __daisy_tt__6->address()});
                                units_done += units_on_core;
                            }
                        }

                        tt::tt_metal::EnqueueProgram(tt_device->command_queue(0), tt_program, 1);

                        double tt_num_cores_used = static_cast<double>(num_cores);
                        double tt_num_cores_available = static_cast<double>((tt_full_grid.x * tt_full_grid.y));
                        double tt_cores_used_rel = tt_num_cores_used / tt_num_cores_available;
                        double tt_work_units_per_core = static_cast<double>(units_done) / tt_num_cores_available;

                    }
                    __daisy_tt_d2h_transfer(
                    	tt_device,	0,
                    	__daisy_tt__6, _6, 
                    	4*_1,
                    	1
                    );
            }
            else if((_24 == false))
            {
                    __daisy_tt__6 = tt::tt_metal::CreateBuffer({
                        .device = tt_device, 
                        .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_1) / (4096))),
                        .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
                        .buffer_type = tt::tt_metal::BufferType::DRAM
                    });
                    __daisy_tt_h2d_transfer(
                    	tt_device, 
                    	0, 
                    	__daisy_tt__6, _6, 
                    	4*_1,
                    	1
                    );
                    __daisy_tt__5 = tt::tt_metal::CreateBuffer({
                        .device = tt_device, 
                        .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_1) / (4096))),
                        .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
                        .buffer_type = tt::tt_metal::BufferType::DRAM
                    });
                    __daisy_tt_h2d_transfer(
                    	tt_device, 
                    	0, 
                    	__daisy_tt__5, _5, 
                    	4*_1,
                    	1
                    );
                    __daisy_tt__3 = tt::tt_metal::CreateBuffer({
                        .device = tt_device, 
                        .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_1) / (4096))),
                        .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
                        .buffer_type = tt::tt_metal::BufferType::DRAM
                    });
                    __daisy_tt_h2d_transfer(
                    	tt_device, 
                    	0, 
                    	__daisy_tt__3, _3, 
                    	4*_1,
                    	1
                    );
                    {
                        tt::tt_metal::Program tt_program;
                        tt::tt_metal::CoreCoord tt_full_grid = tt_device->compute_with_storage_grid_size();
                        auto [num_cores, all_cores, main_cores, remainder_cores, units_per_main, units_per_remainder] = tt::tt_metal::split_work_to_cores(tt_full_grid, ((1023 + _1) / (1024)));
                        auto tt_cb_0_config = tt::tt_metal::CircularBufferConfig(8192, {{0, tt::DataFormat::Float32}})
                        	.set_page_size(0, 4096);
                        auto tt_cb_0 = tt::tt_metal::CreateCircularBuffer(tt_program, all_cores, tt_cb_0_config);
                        auto tt_cb_1_config = tt::tt_metal::CircularBufferConfig(8192, {{1, tt::DataFormat::Float32}})
                        	.set_page_size(1, 4096);
                        auto tt_cb_1 = tt::tt_metal::CreateCircularBuffer(tt_program, all_cores, tt_cb_1_config);
                        auto tt_cb_2_config = tt::tt_metal::CircularBufferConfig(8192, {{2, tt::DataFormat::Float32}})
                        	.set_page_size(2, 4096);
                        auto tt_cb_2 = tt::tt_metal::CreateCircularBuffer(tt_program, all_cores, tt_cb_2_config);
                        std::vector<uint32_t> compile_args_k0, rt_common_args_k0;
                        tt::tt_metal::TensorAccessorArgs(__daisy_tt__3).append_to(compile_args_k0, rt_common_args_k0);
                        tt::tt_metal::TensorAccessorArgs(__daisy_tt__5).append_to(compile_args_k0, rt_common_args_k0);
                        tt::tt_metal::TensorAccessorArgs(__daisy_tt__6).append_to(compile_args_k0, rt_common_args_k0);
                        compile_args_k0.insert(compile_args_k0.end(), {});
                        auto kernel_movRd_0_0 = tt::tt_metal::CreateKernel(tt_program, "/home/ramon/git/OpenFOAM-TT/src/lduMatrix/lduMatrix/sdfg_3_tenstorrent_kernel_657.combined.cpp", all_cores, tt::tt_metal::ReaderDataMovementConfig(compile_args_k0, {}));
                        tt::tt_metal::SetCommonRuntimeArgs(tt_program, kernel_movRd_0_0, rt_common_args_k0);

                        uint32_t units_done = 0;
                        for (auto& tt_range : all_cores.ranges()) {
                            for (auto& core : tt_range) {
                                uint32_t units_on_core = 0;
                                if (main_cores.contains(core)) {
                                	units_on_core = units_per_main;
                                } else if (remainder_cores.contains(core)) {
                                	units_on_core = units_per_remainder;
                                } else {
                                	TT_ASSERT(false, "Core not in specified core ranges");
                                }
                                tt::tt_metal::SetRuntimeArgs(tt_program, kernel_movRd_0_0, core, {units_done, units_on_core, static_cast<uint32_t>(_1), reinterpret_cast<uint32_t&>(_2), reinterpret_cast<uint32_t&>(_4), __daisy_tt__3->address(), __daisy_tt__5->address(), __daisy_tt__6->address()});
                                units_done += units_on_core;
                            }
                        }

                        tt::tt_metal::EnqueueProgram(tt_device->command_queue(0), tt_program, 1);

                        double tt_num_cores_used = static_cast<double>(num_cores);
                        double tt_num_cores_available = static_cast<double>((tt_full_grid.x * tt_full_grid.y));
                        double tt_cores_used_rel = tt_num_cores_used / tt_num_cores_available;
                        double tt_work_units_per_core = static_cast<double>(units_done) / tt_num_cores_available;

                    }
                    __daisy_tt_d2h_transfer(
                    	tt_device,	0,
                    	__daisy_tt__6, _6, 
                    	4*_1,
                    	1
                    );
            }
    }
    return ;

}
