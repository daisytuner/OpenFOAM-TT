cmake_minimum_required(VERSION 3.16)

project(daisy-tt-rt VERSION 0.0.1 DESCRIPTION "A library of predefined tenstorrent kernels and utilities for handling various OpenFOAM structures and operations")

add_library(daisy_tt_rt STATIC
    src/tenstorrent/kernel_launcher.cpp
    src/tenstorrent/ttLduData.cpp
)

add_library(OpenFOAM::daisy_tt_rt ALIAS daisy_tt_rt)

target_compile_features(daisy_tt_rt PUBLIC cxx_std_20)

target_compile_options(daisy_tt_rt PRIVATE
    -fPIC
)

target_compile_definitions(daisy_tt_rt PUBLIC
    SPDLOG_FMT_EXTERNAL
)

target_link_directories(daisy_tt_rt PUBLIC
    $ENV{TT_METAL_INSTALL_DIR}/lib
)

target_link_libraries(daisy_tt_rt PUBLIC
    tt_metal
    fmt
)

target_include_directories(daisy_tt_rt PUBLIC
    src/tenstorrent
    $ENV{TT_METAL_INSTALL_DIR}/include
    $ENV{TT_METAL_INSTALL_DIR}/include/metalium-thirdparty
    $ENV{TT_METAL_INSTALL_DIR}/boost-src/libs/container/include
    $ENV{TT_METAL_INSTALL_DIR}/boost-src/libs/assert/include
    $ENV{TT_METAL_INSTALL_DIR}/boost-src/libs/config/include
    $ENV{TT_METAL_INSTALL_DIR}/boost-src/libs/intrusive/include
    $ENV{TT_METAL_INSTALL_DIR}/boost-src/libs/move/include
    $ENV{TT_METAL_INSTALL_DIR}/boost-src/libs/core/include
    $ENV{TT_METAL_INSTALL_DIR}/boost-src/libs/static_assert/include
    $ENV{TT_METAL_INSTALL_DIR}/boost-src/libs/throw_exception/include
)