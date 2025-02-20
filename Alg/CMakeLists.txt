# 生成库文件
add_library(alg STATIC
    QKDSim.cpp
    Demand.cpp
    KeyManager.cpp
    Link.cpp
    NetEvent.cpp
    Network.cpp
    Node.cpp
    RelayPath.cpp
    stdafx.cpp
    Route/BfsStrategy.cpp
    Route/KeyRateStrategy.cpp
    Route/BidBfsStrategy.cpp
    Route/RouteFactory.cpp
    Route/KeyRateFibStrategy.cpp
    # main_new.cpp
)

# 设置包含目录
target_include_directories(alg PUBLIC ${CMAKE_SOURCE_DIR})