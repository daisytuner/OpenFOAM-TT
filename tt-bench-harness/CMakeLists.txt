
add_executable(tt_bench_harness
    bench_mat_add.cpp
)

target_link_libraries(tt_bench_harness OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)

target_include_directories(tt_bench_harness PRIVATE
    ./include
)

add_executable(tt_bench_harness_Amul
    bench_ldu_Amul.cpp
)

target_link_libraries(tt_bench_harness_Amul OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)

target_include_directories(tt_bench_harness_Amul PRIVATE
    ./include
)