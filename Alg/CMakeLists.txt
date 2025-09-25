# 生成库文件
set(OATPP_INCLUDE_DIR /usr/local/include/oatpp-1.3.0/oatpp)
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
    Route/CustomRouteStrategy.cpp 
    Route/RouteFactory.cpp
    Route/KeyRateFibStrategy.cpp
    Route/OspfStrategy.cpp
    Route/BgpStrategy.cpp
    Sched/AvgStrategy.cpp
    Sched/CustomSchedStrategy.cpp
    Sched/MinStrategy.cpp
    Sched/SchedFactory.cpp
    # main_new.cpp
)

# 设置包含目录
target_include_directories(alg PUBLIC ${CMAKE_SOURCE_DIR} ${OATPP_INCLUDE_DIR})
target_link_libraries(alg PUBLIC dl)