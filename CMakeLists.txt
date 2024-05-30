cmake_minimum_required(VERSION 2.8)
project(ServerClient)

#设置编译模式
set(CMAKE_BUILD_TYPE Release)
#设置CPP11标准
set(CMAKE_CXX_STANDARD 11)
#设置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#设置库文件的输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#包含头文件
include_directories(
    ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(test)