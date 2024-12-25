cmake_minimum_required(VERSION 3.10)

# Project name
project(QKDSim)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the path to the Alg source files
set(ALG_SRC_DIR ${CMAKE_SOURCE_DIR}/Alg)
set(ROUTE_SRC_DIR ${ALG_SRC_DIR}/Route)

# Include directories for Alg and Route
include_directories(
    ${ALG_SRC_DIR}
    ${ROUTE_SRC_DIR}
)

# Set the source files for the entire project
set(ALG_SRC
    ${ALG_SRC_DIR}/Demand.cpp
    ${ALG_SRC_DIR}/KeyManager.cpp
    ${ALG_SRC_DIR}/Link.cpp
    ${ALG_SRC_DIR}/NetEvent.cpp
    ${ALG_SRC_DIR}/Network.cpp
    ${ALG_SRC_DIR}/Node.cpp
    ${ALG_SRC_DIR}/RelayPath.cpp
    ${ROUTE_SRC_DIR}/BfsStrategy.cpp
    ${ROUTE_SRC_DIR}/RouteFactory.cpp
)

# Add source files for other parts of your project (if applicable)
# set(GUI_SRC
#     ${CMAKE_SOURCE_DIR}/GUI/qkdsim.cpp
#     ${CMAKE_SOURCE_DIR}/GUI/progressbar.cpp
#     ${CMAKE_SOURCE_DIR}/GUI/showqkdsim.cpp
# )

# Test target source file
set(TEST_SRC
    ${CMAKE_SOURCE_DIR}/Test/test_route.cpp
)

# Define the main executable for the project
# add_executable(qkdsim ${ALG_SRC} ${GUI_SRC} ${TEST_SRC})

# Link libraries if needed
# target_link_libraries(qkdsim <your_libraries>)

# If you want to separate the test executable from the main one:
add_executable(test_route ${TEST_SRC} ${ALG_SRC})

# Link libraries for the test (if needed)
# target_link_libraries(test_route <your_libraries>)
