cmake_minimum_required(VERSION 3.10)
project(my_muduo)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

# 头文件目录: include/ (内层 mymuduo 为子目录)
include_directories(${PROJECT_SOURCE_DIR}/include/mymuduo)

file(GLOB SRC_LIST "${PROJECT_SOURCE_DIR}/src/*.cc")
add_library(my_muduo SHARED ${SRC_LIST})