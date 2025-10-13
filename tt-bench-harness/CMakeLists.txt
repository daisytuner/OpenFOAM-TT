

add_executable(tt_bench_harness_mat_add
    bench_mat_add.cpp
)

target_link_libraries(tt_bench_harness_mat_add PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)

target_include_directories(tt_bench_harness_mat_add PRIVATE
    ./include
)
if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_harness_mat_add PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_harness_mat_add PRIVATE
    daisy_rtl arg_capture_io
  )
endif()

add_executable(tt_bench_harness_mat_neg
    bench_mat_neg.cpp
)

target_link_libraries(tt_bench_harness_mat_neg PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)

target_include_directories(tt_bench_harness_mat_neg PRIVATE
    ./include
)

if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_harness_mat_neg PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_harness_mat_neg PRIVATE
    daisy_rtl arg_capture_io
  )
endif()

add_executable(tt_bench_dense_Amul
    bench_ldu_Amul_real.cpp
)

target_link_libraries(tt_bench_dense_Amul PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)

target_include_directories(tt_bench_dense_Amul PRIVATE
    ./include
)

if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_dense_Amul PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_dense_Amul PRIVATE
    daisy_rtl arg_capture_io
  )
endif()

add_executable(tt_bench_dense_Amul_test
    bench_ldu_Amul.cpp
)

target_link_libraries(tt_bench_dense_Amul_test PUBLIC OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)

target_include_directories(tt_bench_dense_Amul_test PRIVATE
    ./include
)

if (ENABLE_DAISY_RTL)
  target_compile_definitions(tt_bench_dense_Amul_test PRIVATE
    ENABLE_DAISY_RTL
  )
  target_link_libraries(tt_bench_dense_Amul_test PRIVATE
    daisy_rtl arg_capture_io
  )
endif()
