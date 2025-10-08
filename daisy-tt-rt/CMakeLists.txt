cmake_minimum_required(VERSION 3.16)

project(daisy-tt-rt VERSION 0.0.1 DESCRIPTION "A library of predefined tenstorrent kernels and utilities for handling various OpenFOAM structures and operations")

add_library(daisy_tt_rt STATIC
    src/tenstorrent/buffer_pool.cpp
    src/tenstorrent/dense_matBinOp.cpp
    src/tenstorrent/dense_matMul.cpp
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

set(KERNELS_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/tenstorrent-kernels")
set(KERNELS_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen_kernels")
file(GLOB tenstorrent_kernel_subdirs RELATIVE "${KERNELS_BASE_DIR}" "${KERNELS_BASE_DIR}/*")

message("### Processing subdirs in ${KERNELS_BASE_DIR} ${tenstorrent_kernel_subdirs}")

set(gen_cpp_files "")
foreach(subdir ${tenstorrent_kernel_subdirs})
    if(IS_DIRECTORY "${KERNELS_BASE_DIR}/${subdir}")
        message("Processing tenstorrent kernel subdir: ${subdir}")
        file(GLOB cpp_files RELATIVE "${KERNELS_BASE_DIR}/${subdir}" "${KERNELS_BASE_DIR}/${subdir}/*.cpp")
        set(gen_cpp "${KERNELS_OUTPUT_DIR}/gen_kernels_${subdir}.hpp")
        file(WRITE "${gen_cpp}" "// Auto-generated kernel source file for ${subdir}\n")
        foreach(cpp_file ${cpp_files})
            get_filename_component(var_name "${cpp_file}" NAME_WE)
            set(var_name "tt_kernel_${subdir}_${var_name}")
            # Read file contents
            file(READ "${KERNELS_BASE_DIR}/${subdir}/${cpp_file}" file_content)
            # Write as a C++ const char* variable
            file(APPEND "${gen_cpp}" "const inline char* ${var_name} = R\"MULTI(\n${file_content}\n)MULTI\";\n")
        endforeach()
        list(APPEND gen_cpp_files "${gen_cpp}")
    endif()
endforeach()

add_custom_target(generate_gen_cpp_files ALL DEPENDS ${gen_cpp_files})
add_dependencies(daisy_tt_rt generate_gen_cpp_files)
target_include_directories(daisy_tt_rt PRIVATE "${KERNELS_OUTPUT_DIR}")