add_executable(tt_bench_dense_mat_add.out
    bench_dense_mat_add.cpp
)
target_link_libraries(tt_bench_dense_mat_add.out PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)
target_include_directories(tt_bench_dense_mat_add.out PRIVATE
    ./include
)
if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_dense_mat_add.out PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_dense_mat_add.out PRIVATE
    daisy_rtl arg_capture_io
  )
endif()

add_executable(tt_bench_dense_mat_sub.out
    bench_dense_mat_sub.cpp
)
target_link_libraries(tt_bench_dense_mat_sub.out PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)
target_include_directories(tt_bench_dense_mat_sub.out PRIVATE
    ./include
)
if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_dense_mat_sub.out PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_dense_mat_sub.out PRIVATE
    daisy_rtl arg_capture_io
  )
endif()

add_executable(tt_bench_dense_mat_neg.out
    bench_dense_mat_neg.cpp
)
target_link_libraries(tt_bench_dense_mat_neg.out PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)
target_include_directories(tt_bench_dense_mat_neg.out PRIVATE
    ./include
)
if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_dense_mat_neg.out PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_dense_mat_neg.out PRIVATE
    daisy_rtl arg_capture_io
  )
endif()

add_executable(tt_bench_dense_Amul.out
    bench_dense_Amul_real.cpp
    ref_Amul.cpp
)
target_link_libraries(tt_bench_dense_Amul.out PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)
target_include_directories(tt_bench_dense_Amul.out PRIVATE
    ./include
)
if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_dense_Amul.out PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_dense_Amul.out PRIVATE
    daisy_rtl arg_capture_io
  )
endif()

add_executable(tt_bench_dense_Amul_test.out
    bench_dense_Amul.cpp
    ref_Amul.cpp
)
target_link_libraries(tt_bench_dense_Amul_test.out PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)
target_include_directories(tt_bench_dense_Amul_test.out PRIVATE
    ./include
)
if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_dense_Amul_test.out PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_dense_Amul_test.out PRIVATE
    daisy_rtl arg_capture_io
  )
endif()


add_executable(tt_bench_ellpack_Amul.out
    bench_ellpack_Amul.cpp
    ref_Amul.cpp
)
target_link_libraries(tt_bench_ellpack_Amul.out PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)
target_include_directories(tt_bench_ellpack_Amul.out PRIVATE
    ./include
)
if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_ellpack_Amul.out PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_ellpack_Amul.out PRIVATE
    daisy_rtl arg_capture_io
  )
endif()

add_executable(tt_bench_ellpack_Amul_no_hw.out
    bench_ellpack_Amul.cpp
    ref_Amul.cpp
)
target_link_libraries(tt_bench_ellpack_Amul_no_hw.out PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)
target_include_directories(tt_bench_ellpack_Amul_no_hw.out PRIVATE
    ./include
)
target_compile_definitions(tt_bench_ellpack_Amul_no_hw.out PUBLIC
    ELLPACK_HW_IMPL=EllpackHwImpl::None
)
if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_ellpack_Amul_no_hw.out PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_ellpack_Amul_no_hw.out PRIVATE
    daisy_rtl arg_capture_io
  )
endif()

add_executable(tt_bench_ellpack_Amul_cavity200.out
    bench_ellpack_Amul_cavity200.cpp
    ref_Amul.cpp
)
target_link_libraries(tt_bench_ellpack_Amul_cavity200.out PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)
target_include_directories(tt_bench_ellpack_Amul_cavity200.out PRIVATE
    ./include
)
target_compile_definitions(tt_bench_ellpack_Amul_cavity200.out PUBLIC
    ELLPACK_HW_IMPL=EllpackHwImpl::None
)
if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_ellpack_Amul_cavity200.out PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_ellpack_Amul_cavity200.out PRIVATE
    daisy_rtl arg_capture_io
  )
endif()

add_executable(tt_bench_ldu_Amul.out
    bench_ldu_Amul_real.cpp
    ref_Amul.cpp
)
target_link_libraries(tt_bench_ldu_Amul.out PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)
target_include_directories(tt_bench_ldu_Amul.out PRIVATE
    ./include
)
if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_ldu_Amul.out PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_ldu_Amul.out PRIVATE
    daisy_rtl arg_capture_io
  )
endif()
