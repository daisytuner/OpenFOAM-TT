# Test Case: lduMatrixATmul_SumA
add_executable(lduMatrixATmul_SumA
    lduMatrixATmul_SumA.cpp
)
target_link_libraries(lduMatrixATmul_SumA PUBLIC OpenFOAM::OpenFOAM z)
target_compile_features(lduMatrixATmul_SumA PUBLIC cxx_std_20)
target_include_directories(lduMatrixATmul_SumA PRIVATE
    ${OPENFOAM_HEADERS}
)
target_compile_definitions(lduMatrixATmul_SumA PRIVATE
    ${OPENFOAM_COMPILE_DEFINITIONS}
)
target_compile_options(lduMatrixATmul_SumA PRIVATE
    ${OPENFOAM_COMPILE_OPTIONS}
)

# Test Case: lduMatrixOperations_AddAssign_asymm
add_executable(lduMatrixOperations_AddAssign_asymm
    lduMatrixOperations_AddAssign_asymm.cpp
)
target_link_libraries(lduMatrixOperations_AddAssign_asymm PUBLIC OpenFOAM::OpenFOAM z)
target_compile_features(lduMatrixOperations_AddAssign_asymm PUBLIC cxx_std_20)
target_include_directories(lduMatrixOperations_AddAssign_asymm PRIVATE
    ${OPENFOAM_HEADERS}
)
target_compile_definitions(lduMatrixOperations_AddAssign_asymm PRIVATE
    ${OPENFOAM_COMPILE_DEFINITIONS}
)
target_compile_options(lduMatrixOperations_AddAssign_asymm PRIVATE
    ${OPENFOAM_COMPILE_OPTIONS}
)

# Test Case: lduMatrixOperations_AddAssign_symm
add_executable(lduMatrixOperations_AddAssign_symm
    lduMatrixOperations_AddAssign_symm.cpp
)
target_link_libraries(lduMatrixOperations_AddAssign_symm PUBLIC OpenFOAM::OpenFOAM z)
target_compile_features(lduMatrixOperations_AddAssign_symm PUBLIC cxx_std_20)
target_include_directories(lduMatrixOperations_AddAssign_symm PRIVATE
    ${OPENFOAM_HEADERS}
)
target_compile_definitions(lduMatrixOperations_AddAssign_symm PRIVATE
    ${OPENFOAM_COMPILE_DEFINITIONS}
)
target_compile_options(lduMatrixOperations_AddAssign_symm PRIVATE
    ${OPENFOAM_COMPILE_OPTIONS}
)

