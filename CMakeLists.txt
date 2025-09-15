cmake_minimum_required(VERSION 3.26)

project(OpenFOAM-TT VERSION "1.0.0" LANGUAGES CXX)

set(OPENFOAM_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/OpenFOAM-dev/src)

add_library(OpenFOAM SHARED
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/error/error.C
)
target_compile_features(OpenFOAM PUBLIC cxx_std_17)

target_include_directories(OpenFOAM PUBLIC
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/lnInclude
  ${OPENFOAM_SRC_DIR}/OpenFOAM/lnInclude
)

target_precompile_headers(OpenFOAM PRIVATE "${OPENFOAM_SRC_DIR}/OpenFOAM/include/OSspecific.H")

target_compile_definitions(OpenFOAM PUBLIC
    WM_PRECISION_OPTION=SP
    WM_LABEL_SIZE=32
    WM_SP
    NoRepository
)

add_subdirectory(tests)
