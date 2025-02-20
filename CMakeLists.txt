# # 设置 CMake 最低版本
# cmake_minimum_required(VERSION 3.10)

# # 设置项目名称
# project(DistriQKDSim)

# # 设置 C++17 标准
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # oatpp安装路径
# set(OATPP_PATH /usr/local/include/oatpp-1.4.0/oatpp)

# # 设置输出目录
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# # 包含头文件目录
# include_directories(
#     ${PROJECT_SOURCE_DIR}/Alg  # 添加 Alg 文件夹下的头文件路径
#     ${PROJECT_SOURCE_DIR}/
#     ${PROJECT_SOURCE_DIR}/Route
#     ${OATPP_PATH}
# )


# # 设置源文件
# set(SOURCES
#     Alg/QKDSim.cpp
#     Alg/Demand.cpp
#     Alg/KeyManager.cpp
#     Alg/Link.cpp
#     Alg/NetEvent.cpp
#     Alg/Network.cpp
#     Alg/Node.cpp
#     Alg/RelayPath.cpp
#     Alg/stdafx.cpp
#     # Alg/main_new.cpp
#     Alg/Route/BfsStrategy.cpp
#     Alg/Route/KeyRateStrategy.cpp
#     Alg/Route/BidBfsStrategy.cpp
#     Alg/Route/RouteFactory.cpp
#     Alg/Route/KeyRateFibStrategy.cpp
#     Web/main.cpp
# )

# # 如果项目中有其他的头文件，可以显式地列出
# set(HEADERS
#     Alg/QKDSim.h
#     Alg/Demand.h
#     Alg/InputOutput.h
#     Alg/KeyManager.h
#     Alg/Link.h
#     Alg/NetEvent.h
#     Alg/Network.h
#     Alg/Node.h
#     Alg/RelayPath.h
#     Alg/stdafx.h
#     Alg/targetver.h
#     Alg/Route/BfsStrategy.h
#     Alg/Route/KeyRateFibStrategy.h
#     Alg/Route/BidBfsStrategy.h
#     Alg/Route/KeyRateStrategy.h
#     Alg/Route/FibonacciHeap.h
#     Alg/Route/RouteFactory.h
# )

# # 创建可执行文件
# add_executable(DistriQKDSim ${SOURCES})

# # 如果有额外的库依赖，可以在这里链接
# # 例如，链接 pthread 库（Linux 下可能需要）
# find_package(Threads REQUIRED)
# target_link_libraries(DistriQKDSim PRIVATE Threads::Threads)

# # 如果你有自定义的库，链接它们：
# # target_link_libraries(DistriQKDSim PRIVATE your_library_name)


cmake_minimum_required(VERSION 3.10)
project(DistriQKDSim)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)


# 添加子目录
add_subdirectory(Alg)
add_subdirectory(Web)
