cmake_minimum_required(VERSION 3.26)

project(OpenFOAM_DOCCPlugin VERSION 0.0.1 DESCRIPTION "A plugin that provides OpenFOAM kernels for docc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(sdfglib CONFIG REQUIRED)

add_library(OpenFOAM_DOCCPlugin SHARED
    plugin.cpp
)
target_link_libraries(OpenFOAM_DOCCPlugin PRIVATE sdfglib::sdfglib)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  get_filename_component(COMPILER_NAME ${CMAKE_CXX_COMPILER} NAME)
  if(COMPILER_NAME MATCHES "docc")
    target_compile_options(OpenFOAM_DOCCPlugin PRIVATE
        -docc-noop
    )
  endif()

endif()
