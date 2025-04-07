

add_library(config STATIC
    ConfigReader.cpp
)

target_include_directories(config PUBLIC ${CMAKE_SOURCE_DIR})