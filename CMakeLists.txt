cmake_minimunum_required(VERSION 3.0)
project(my_muduo)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# 设置C++17标准
set(CMAKE_CXX_STANDARD 17)
# 编译生成的动态库放在lib下
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# 设置调试信息,启动C++17标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -fPIC")
# 定义参与编译的源代码文件
aux_source_directory(. SRC_LIST)
#  编译生成动态库
add_library(my_muduo ${SRC_LIST})