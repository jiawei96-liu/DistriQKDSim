# 设置 CMake 最低版本
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(DistriQKDSim)

# 设置 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 设置源文件
set(SOURCES
    Alg/QKDSim.cpp
    Alg/Demand.cpp
    Alg/KeyManager.cpp
    Alg/Link.cpp
    Alg/NetEvent.cpp
    Alg/Network.cpp
    Alg/Node.cpp
    Alg/RelayPath.cpp
    Alg/stdafx.cpp
    Alg/main_new.cpp
    Alg/Route/BfsStrategy.cpp
    Alg/Route/KeyRateStrategy.cpp
    Alg/Route/BidBfsStrategy.cpp
    Alg/Route/RouteFactory.cpp
    Alg/Route/KeyRateFibStrategy.cpp
)

# 设置头文件
set(HEADERS
    Alg/QKDSim.h
    Alg/Demand.h
    Alg/InputOutput.h
    Alg/KeyManager.h
    Alg/Link.h
    Alg/NetEvent.h
    Alg/Network.h
    Alg/Node.h
    Alg/RelayPath.h
    Alg/stdafx.h
    Alg/targetver.h
    Alg/Route/BfsStrategy.h
    Alg/Route/KeyRateFibStrategy.h
    Alg/Route/BidBfsStrategy.h
    Alg/Route/KeyRateStrategy.h
    Alg/Route/FibonacciHeap.h
    Alg/Route/RouteFactory.h
)

# 创建可执行文件
add_executable(DistriQKDSim ${SOURCES})

# 包含头文件目录
target_include_directories(DistriQKDSim PRIVATE
    ${PROJECT_SOURCE_DIR}/Alg  # 添加 Alg 文件夹下的头文件路径
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/Route
)

# 链接线程库
find_package(Threads REQUIRED)
target_link_libraries(DistriQKDSim PRIVATE Threads::Threads)

# 添加调试信息
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(DistriQKDSim PRIVATE -g -O0)  # 启用调试符号，禁用优化
else()
    target_compile_options(DistriQKDSim PRIVATE -O2)  # 启用优化
endif()