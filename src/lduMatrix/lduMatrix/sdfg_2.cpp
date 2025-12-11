#include "sdfg_2.h"
extern "C" void _ZdlPvm(void* , long long );
extern "C" void _ZNK4Foam9lduMatrix22updateMatrixInterfacesERKNS_10FieldFieldINS_5FieldEfEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEERKNS2_IfEERSC_h(void* , void* , void* , void* , void* , signed char );
extern "C" void* _ZNK4Foam9lduMatrix5upperEv(void* );
extern "C" void* _ZNK4Foam9lduMatrix4diagEv(void* );
extern "C" void _ZdaPv(void* );
extern "C" void _ZNK4Foam9lduMatrix20initMatrixInterfacesERKNS_10FieldFieldINS_5FieldEfEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEERKNS2_IfEERSC_h(void* , void* , void* , void* , void* , signed char );
extern "C" void* _ZNK4Foam9lduMatrix5lowerEv(void* );
extern "C" void* _ZNK4Foam3tmpINS_5FieldIfEEEclEv(void* );

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


extern "C" void sdfg_2_func(void* _0, void* _1, void* _2, void* _3, void* _4, signed char _5)
{
tt::tt_metal::IDevice* tt_device = daisy::tenstorrent::daisy_get_tt_device(0);

std::shared_ptr<tt::tt_metal::Buffer> __daisy_tt__14;
std::shared_ptr<tt::tt_metal::Buffer> __daisy_tt__11;
int _98 __attribute__((aligned(4)));
void* _95 __attribute__((aligned(8)));
bool _92 __attribute__((aligned(1)));
int _72 __attribute__((aligned(4)));
long long _66 __attribute__((aligned(8)));
void* _16 __attribute__((aligned(8)));
void* _12 __attribute__((aligned(8)));
void* _102 __attribute__((aligned(8)));
void* _101 __attribute__((aligned(8)));
void* _41 __attribute__((aligned(8)));
void* _10 __attribute__((aligned(8)));
void* _24 __attribute__((aligned(8)));
float _75 __attribute__((aligned(4)));
void* _31 __attribute__((aligned(8)));
void* _14 __attribute__((aligned(8)));
void* _21 __attribute__((aligned(8)));
void* _15 __attribute__((aligned(8)));
int _107 __attribute__((aligned(4)));
void* _94 __attribute__((aligned(8)));
void* _22 __attribute__((aligned(8)));
void* _37 __attribute__((aligned(8)));
float _70 __attribute__((aligned(4)));
void* _17 __attribute__((aligned(8)));
void* _28 __attribute__((aligned(8)));
void* _36 __attribute__((aligned(8)));
void* _11 __attribute__((aligned(8)));
void* _9 __attribute__((aligned(8)));
void* _20 __attribute__((aligned(8)));
void* _19 __attribute__((aligned(8)));
void* _39 __attribute__((aligned(8)));
void* _23 __attribute__((aligned(8)));
void* _26 __attribute__((aligned(8)));
void* _29 __attribute__((aligned(8)));
void* _34 __attribute__((aligned(8)));
void* _30 __attribute__((aligned(8)));
void* _32 __attribute__((aligned(8)));
void* _7 __attribute__((aligned(8)));
void* _60 __attribute__((aligned(8)));
void* _8 __attribute__((aligned(8)));
void* _33 __attribute__((aligned(8)));
void* _35 __attribute__((aligned(8)));
std::shared_ptr<tt::tt_metal::Buffer> __daisy_tt__8;
void* _38 __attribute__((aligned(8)));
void* _40 __attribute__((aligned(8)));
void* _27 __attribute__((aligned(8)));
void* _42 __attribute__((aligned(8)));
int _77 __attribute__((aligned(4)));
void* _43 __attribute__((aligned(8)));
void* _13 __attribute__((aligned(8)));
int _45 __attribute__((aligned(4)));
long long _49 __attribute__((aligned(8)));
long long _49_tile0 __attribute__((aligned(8)));
void* _18 __attribute__((aligned(8)));
void* _25 __attribute__((aligned(8)));
int _62 __attribute__((aligned(4)));
    {
        _7 = &(reinterpret_cast<signed char *>(_1))[16];
    }
    {
        _8 = *(reinterpret_cast<void* *>(_7));
    }
    {
        void* _arg0 = (reinterpret_cast<void* >(_2));
        void* _ret = (reinterpret_cast<void* >(_9));

        _ret = _ZNK4Foam3tmpINS_5FieldIfEEEclEv(_arg0);

        _9 = _ret;
        _2 = _arg0;
    }
    {
        void* _arg4 = (reinterpret_cast<void* >(_1));
        void* _arg0 = (reinterpret_cast<void* >(_0));
        void* _arg1 = (reinterpret_cast<void* >(_3));
        void* _arg2 = (reinterpret_cast<void* >(_4));
        void* _arg3 = (reinterpret_cast<void* >(_9));
        signed char _arg5 = _5;

        _ZNK4Foam9lduMatrix20initMatrixInterfacesERKNS_10FieldFieldINS_5FieldEfEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEERKNS2_IfEERSC_h(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);

        _3 = _arg1;
        _4 = _arg2;
        _1 = _arg4;
        _0 = _arg0;
        _9 = _arg3;
    }
    {
        _10 = &(reinterpret_cast<signed char *>(_9))[16];
    }
    {
        _11 = *(reinterpret_cast<void* *>(_10));
    }
    {
        void* _arg0 = (reinterpret_cast<void* >(_0));
        void* _ret = (reinterpret_cast<void* >(_12));

        _ret = _ZNK4Foam9lduMatrix4diagEv(_arg0);

        _12 = _ret;
        _0 = _arg0;
    }
    {
        _13 = &(reinterpret_cast<signed char *>(_12))[16];
    }
    {
        _14 = *(reinterpret_cast<void* *>(_13));
    }
    {
        _15 = *(reinterpret_cast<void* *>(_0));
    }
    {
        _16 = *(reinterpret_cast<void* *>(_15));
    }
    {
        _17 = &(reinterpret_cast<signed char *>(_16))[32];
    }
    {
        _18 = *(reinterpret_cast<void* *>(_17));
    }
    {
        void* _arg0 = (reinterpret_cast<void* >(_15));
        void* _ret = (reinterpret_cast<void* >(_19));

        _ret = reinterpret_cast<void*  (*)(void* )>(_18)(_arg0);

        _15 = _arg0;
        _19 = _ret;
    }
    {
        _20 = *(reinterpret_cast<void* *>(_19));
    }
    {
        _21 = &(reinterpret_cast<signed char *>(_20))[24];
    }
    {
        _22 = *(reinterpret_cast<void* *>(_21));
    }
    {
        void* _arg0 = (reinterpret_cast<void* >(_19));
        void* _ret = (reinterpret_cast<void* >(_23));

        _ret = reinterpret_cast<void*  (*)(void* )>(_22)(_arg0);

        _23 = _ret;
        _19 = _arg0;
    }
    {
        _24 = &(reinterpret_cast<signed char *>(_23))[8];
    }
    {
        _25 = *(reinterpret_cast<void* *>(_24));
    }
    {
        _26 = *(reinterpret_cast<void* *>(_0));
    }
    {
        _27 = *(reinterpret_cast<void* *>(_26));
    }
    {
        _28 = &(reinterpret_cast<signed char *>(_27))[32];
    }
    {
        _29 = *(reinterpret_cast<void* *>(_28));
    }
    {
        void* _arg0 = (reinterpret_cast<void* >(_26));
        void* _ret = (reinterpret_cast<void* >(_30));

        _ret = reinterpret_cast<void*  (*)(void* )>(_29)(_arg0);

        _30 = _ret;
        _26 = _arg0;
    }
    {
        _31 = *(reinterpret_cast<void* *>(_30));
    }
    {
        _32 = &(reinterpret_cast<signed char *>(_31))[16];
    }
    {
        _33 = *(reinterpret_cast<void* *>(_32));
    }
    {
        void* _arg0 = (reinterpret_cast<void* >(_30));
        void* _ret = (reinterpret_cast<void* >(_34));

        _ret = reinterpret_cast<void*  (*)(void* )>(_33)(_arg0);

        _34 = _ret;
        _30 = _arg0;
    }
    {
        _35 = &(reinterpret_cast<signed char *>(_34))[8];
    }
    {
        _36 = *(reinterpret_cast<void* *>(_35));
    }
    {
        void* _arg0 = (reinterpret_cast<void* >(_0));
        void* _ret = (reinterpret_cast<void* >(_37));

        _ret = _ZNK4Foam9lduMatrix5upperEv(_arg0);

        _0 = _arg0;
        _37 = _ret;
    }
    {
        _38 = &(reinterpret_cast<signed char *>(_37))[16];
    }
    {
        _39 = *(reinterpret_cast<void* *>(_38));
    }
    {
        void* _arg0 = (reinterpret_cast<void* >(_0));
        void* _ret = (reinterpret_cast<void* >(_40));

        _ret = _ZNK4Foam9lduMatrix5lowerEv(_arg0);

        _40 = _ret;
        _0 = _arg0;
    }
    {
        _41 = &(reinterpret_cast<signed char *>(_40))[16];
    }
    {
        _42 = *(reinterpret_cast<void* *>(_41));
    }
    {
        void* _arg0 = (reinterpret_cast<void* >(_0));
        void* _ret = (reinterpret_cast<void* >(_43));

        _ret = _ZNK4Foam9lduMatrix4diagEv(_arg0);

        _43 = _ret;
        _0 = _arg0;
    }
    {
        int _in = (reinterpret_cast<int *>(_43))[2];
        int __out;

        __out = _in;

        _45 = __out;
    }
    __daisy_tt__8 = tt::tt_metal::CreateBuffer({
        .device = tt_device, 
        .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_45) / (4096))),
        .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
        .buffer_type = tt::tt_metal::BufferType::DRAM
    });
    __daisy_tt_h2d_transfer(
    	tt_device, 
    	0, 
    	__daisy_tt__8, _8, 
    	4*_45,
    	1
    );
    __daisy_tt__14 = tt::tt_metal::CreateBuffer({
        .device = tt_device, 
        .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_45) / (4096))),
        .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
        .buffer_type = tt::tt_metal::BufferType::DRAM
    });
    __daisy_tt_h2d_transfer(
    	tt_device, 
    	0, 
    	__daisy_tt__14, _14, 
    	4*_45,
    	1
    );
    __daisy_tt__11 = tt::tt_metal::CreateBuffer({
        .device = tt_device, 
        .size = static_cast<tt::tt_metal::DeviceAddr>(4096*((4095 + 4*_45) / (4096))),
        .page_size = static_cast<tt::tt_metal::DeviceAddr>(4096), 
        .buffer_type = tt::tt_metal::BufferType::DRAM
    });
    __daisy_tt_h2d_transfer(
    	tt_device, 
    	0, 
    	__daisy_tt__11, _11, 
    	4*_45,
    	1
    );
    tt::tt_metal::Program tt_program;
    {
        tt::tt_metal::CoreCoord tt_full_grid = tt_device->compute_with_storage_grid_size();
        auto [num_cores, all_cores, main_cores, remainder_cores, units_per_main, units_per_remainder] = tt::tt_metal::split_work_to_cores(tt_full_grid, ((1023 + _45) / (1024)));
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
        tt::tt_metal::TensorAccessorArgs(__daisy_tt__11).append_to(compile_args_k0, rt_common_args_k0);
        tt::tt_metal::TensorAccessorArgs(__daisy_tt__14).append_to(compile_args_k0, rt_common_args_k0);
        tt::tt_metal::TensorAccessorArgs(__daisy_tt__8).append_to(compile_args_k0, rt_common_args_k0);
        compile_args_k0.insert(compile_args_k0.end(), {});
        auto kernel_movRd_0_0 = tt::tt_metal::CreateKernel(tt_program, "/home/ramon/git/OpenFOAM-TT/src/lduMatrix/lduMatrix/sdfg_2_tenstorrent_kernel_944.combined.cpp", all_cores, tt::tt_metal::ReaderDataMovementConfig(compile_args_k0, {}));
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
                tt::tt_metal::SetRuntimeArgs(tt_program, kernel_movRd_0_0, core, {units_done, units_on_core, static_cast<uint32_t>(_45), __daisy_tt__11->address(), __daisy_tt__14->address(), __daisy_tt__8->address()});
                units_done += units_on_core;
            }
        }

        tt::tt_metal::EnqueueProgram(tt_device->command_queue(0), tt_program, 1);

        double tt_num_cores_used = static_cast<double>(num_cores);
        double tt_num_cores_available = static_cast<double>((tt_full_grid.x * tt_full_grid.y));
        double tt_cores_used_rel = tt_num_cores_used / tt_num_cores_available;
        double tt_work_units_per_core = static_cast<double>(units_done) / tt_num_cores_available;

    }
    for (int i = 0; i < _45; ++i) {
        (reinterpret_cast<float*>(_8))[i] = (reinterpret_cast<float*>(_11))[i] * (reinterpret_cast<float*>(_14))[i];
    }
    // __daisy_tt_d2h_transfer(
    // 	tt_device,	0,
    // 	__daisy_tt__8, _8, 
    // 	4*_45,
    // 	1
    // );
    {
        void* _arg0 = (reinterpret_cast<void* >(_0));
        void* _ret = (reinterpret_cast<void* >(_60));

        _ret = _ZNK4Foam9lduMatrix5upperEv(_arg0);

        _60 = _ret;
        _0 = _arg0;
    }
    {
        int _in = (reinterpret_cast<int *>(_60))[2];
        int __out;

        __out = _in;

        _62 = __out;
    }
    for(_66 = 0;_66 < _62;_66 = 1 + _66)
    {
            {
                int _in = (reinterpret_cast<int *>(_36))[_66];
                int __out;

                __out = _in;

                _72 = __out;
            }
            {
                float _in = (reinterpret_cast<float *>(_42))[_66];
                float __out;

                __out = _in;

                _70 = __out;
            }
            {
                int _in = (reinterpret_cast<int *>(_25))[_66];
                int __out;

                __out = _in;

                _77 = __out;
            }
            {
                float _in = (reinterpret_cast<float *>(_11))[_72];
                float __out;

                __out = _in;

                _75 = __out;
            }
            {
                float _in2 = _75;
                float _in1 = _70;
                float _in3 = (reinterpret_cast<float *>(_8))[_77];
                float _out;

                _out = _in1 * _in2 + _in3;

                (reinterpret_cast<float *>(_8))[_77] = _out;
            }
            {
                float _in1 = (reinterpret_cast<float *>(_39))[_66];
                float _in3 = (reinterpret_cast<float *>(_8))[_72];
                float _in2 = (reinterpret_cast<float *>(_11))[_77];
                float _out;

                _out = _in1 * _in2 + _in3;

                (reinterpret_cast<float *>(_8))[_72] = _out;
            }
    }
    {
        void* _arg0 = (reinterpret_cast<void* >(_0));
        void* _arg1 = (reinterpret_cast<void* >(_3));
        void* _arg2 = (reinterpret_cast<void* >(_4));
        void* _arg3 = (reinterpret_cast<void* >(_9));
        signed char _arg5 = _5;
        void* _arg4 = (reinterpret_cast<void* >(_1));

        _ZNK4Foam9lduMatrix22updateMatrixInterfacesERKNS_10FieldFieldINS_5FieldEfEERKNS_8UPtrListIKNS_17lduInterfaceFieldEEERKNS2_IfEERSC_h(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);

        _0 = _arg0;
        _9 = _arg3;
        _4 = _arg2;
        _1 = _arg4;
        _3 = _arg1;
    }
    {
        unsigned int _in2 = 2;
        unsigned int _in1 = (reinterpret_cast<int *>(_2))[0];
        bool __out;

        __out = _in1 < _in2;

        _92 = __out;
    }
    if((_92 != false))
    {
            {
                _94 = &(reinterpret_cast<signed char *>(_2))[8];
            }
            {
                _95 = *(reinterpret_cast<void* *>(_94));
            }
            if((false == ((reinterpret_cast<uintptr_t>(_95)) == (reinterpret_cast<uintptr_t>(nullptr)))))
            {
                    {
                        int _in = (reinterpret_cast<int *>(_95))[0];
                        int __out;

                        __out = _in;

                        _98 = __out;
                    }
                    if((false != (0 == _98)))
                    {
                            {
                                _101 = &(reinterpret_cast<signed char *>(_95))[16];
                            }
                            {
                                _102 = *(reinterpret_cast<void* *>(_101));
                            }
                            if((false == ((reinterpret_cast<uintptr_t>(_102)) == (reinterpret_cast<uintptr_t>(nullptr)))))
                            {
                                    {
                                        void* _arg0 = (reinterpret_cast<void* >(_102));

                                        _ZdaPv(_arg0);

                                        _102 = _arg0;
                                    }
                            }
                            {
                                void* _arg0 = (reinterpret_cast<void* >(_95));
                                long long _arg1 = 24;

                                _ZdlPvm(_arg0, _arg1);

                                _95 = _arg0;
                            }
                    }
                    else if((false == (0 == _98)))
                    {
                        {
                            _107 = -1 + _98;
                        }
                            {
                                int _in = _107;
                                int __out;

                                __out = _in;

                                (reinterpret_cast<int *>(_95))[0] = __out;
                            }
                    }
                    {
                        *(reinterpret_cast<void* *>(_94)) = nullptr;
                    }
            }
    }
    return ;

}
