

add_executable(tt_bench_harness_mat_add
    bench_mat_add.cpp
)

target_link_libraries(tt_bench_harness_mat_add OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)

target_include_directories(tt_bench_harness_mat_add PRIVATE
    ./include
)

add_executable(tt_bench_harness_mat_neg
    bench_mat_neg.cpp
)

target_link_libraries(tt_bench_harness_mat_neg OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)

target_include_directories(tt_bench_harness_mat_neg PRIVATE
    ./include
)