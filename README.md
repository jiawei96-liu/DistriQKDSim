# QKDSim

## 编译运行
```
mkdir build
cd build
cmake ..
make

# 运行
./bin/DistriQKDSim
```

## 路由算法
位于Alg/Route目录

当前默认使用朴素BFS

若需修改默认路由算法
在Network.cpp的Network构造函数里修改

```cpp
CNetwork::CNetwork(void)
{
    // ...
    m_routeFactory=make_unique<route::RouteFactory>(this);
    
    m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_Bfs));    //BFS

    // m_routeStrategy=std::move(m_routeFactory->CreateStrategy(route::RouteType_KeyRateShortestPath));   //keyrate最短路策略

    
    //...
}

```

当前有两种路由策略
```cpp
typedef enum {
    RouteType_Bfs,  //BFS
    RouteType_KeyRateShortestPath,  //密钥速率Dijkstra
} RouteStrategyType;
```

BFS有朴素实现和双向队列实现，Dijkstra有二叉堆实现和Fibonacci堆实现

若要测试不同实现策略，在/Alg/RouteFactory.cpp修改CreateStrategy函数
```cpp
std::unique_ptr<RouteStrategy> RouteFactory::CreateStrategy(RouteStrategyType type) {
    if (type == RouteType_Bfs) {
        // std::cout << "Creating BFS Strategy\n";
        return std::make_unique<BfsStrategy>(net);  //朴素BFS
        // return std::make_unique<BidBfsStrategy>(net);    //双向BFS
    }else if(type==RouteType_KeyRateShortestPath){
        return std::make_unique<KeyRateStrategy>(net);  //二叉堆
        // return std::make_unique<KeyRateFibStrategy>(net);    //斐波那契堆
    }else {
        std::cerr << "Unknown strategy type: " << type << std::endl;
    }
    return nullptr;
}
```