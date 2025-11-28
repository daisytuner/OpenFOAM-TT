cmake_minimum_required(VERSION 3.26)

project(OpenFOAM_DOCCPlugin VERSION 0.0.1 DESCRIPTION "A plugin that provides OpenFOAM kernels for docc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER clang++-19)

find_package(sdfglib CONFIG REQUIRED)

add_library(OpenFOAM_DOCCPlugin SHARED
    plugin.cpp
)
target_link_libraries(OpenFOAM_DOCCPlugin PRIVATE sdfglib::sdfglib)
