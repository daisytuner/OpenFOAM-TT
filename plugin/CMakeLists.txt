cmake_minimum_required(VERSION 3.26)

project(OpenFOAM_DOCCPlugin VERSION 0.0.1 DESCRIPTION "A simple plugin for docc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT TARGET sdfglib::sdfglib)
    find_package(sdfglib CONFIG REQUIRED)
endif()

add_library(OpenFOAM_DOCCPlugin SHARED
    plugin.cpp
    simple_node.cpp
)
target_link_libraries(OpenFOAM_DOCCPlugin PRIVATE sdfglib::sdfglib)
