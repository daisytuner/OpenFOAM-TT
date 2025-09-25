add_executable(lduMatrixATmul_sumA_test
    lduMatrixATmul_sumA_test.cpp
    ${OPENFOAM_SOURCES}
)
target_link_libraries(lduMatrixATmul_sumA_test PUBLIC z)
target_compile_features(lduMatrixATmul_sumA_test PUBLIC cxx_std_20)

target_include_directories(lduMatrixATmul_sumA_test PRIVATE
    ${OPENFOAM_HEADERS}
)

add_compile_definitions(lduMatrixATmul_sumA_test PRIVATE
    ${OPENFOAM_COMPILE_DEFINITIONS}
)

add_compile_options(lduMatrixATmul_sumA_test PRIVATE
    ${OPENFOAM_COMPILE_OPTIONS}
)
