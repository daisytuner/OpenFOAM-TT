
add_executable(tt_bench_dense_mat_add
    bench_mat_add.cpp
)

target_link_libraries(tt_bench_dense_mat_add OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)

target_include_directories(tt_bench_dense_mat_add PRIVATE
    ./include
)

add_executable(tt_bench_dense_Amul
    bench_ldu_Amul.cpp
)

target_link_libraries(tt_bench_dense_Amul OpenFOAM::OpenFOAM OpenFOAM::daisy_tt_rt)

target_include_directories(tt_bench_dense_Amul PRIVATE
    ./include
)