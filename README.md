# QKDSim 
# 1.Web版编译与部署
## 1.1 安装依赖库oatpp-1.3.0版本

前置条件
```
Git
C++ compiler supporting C++ version >= 11.
Make
CMake version >= 3.1
```

安装oatpp-1.3.0
```bash
git clone --branch 1.3.0 --single-branch https://github.com/oatpp/oatpp.git

cd oatpp/

mkdir build && cd build

cmake ..
make install
```

正常会安装在`/usr/local/lib/oatpp-1.3.0/`目录下，若不是则后续编译QKDSim需要修改CMakeLists

## 1.2 编译QKDSim Web版
```bash
mkdir build
cd build
cmake ..
make
```

## 1.3 运行QKDSim Web版
```
cd build #一定要在build目录下运行
./Web/webapp
```

随后访问http://localhost:8080即可

## 1.4 修改部署的IP和端口
`Web/AppComponent.hpp`

```cpp
class AppComponent {
public:
  
  /**
   *  Create ConnectionProvider component which listens on the port
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([] {
    return oatpp::network::tcp::server::ConnectionProvider::createShared({"0.0.0.0", 8080, oatpp::network::Address::IP_4});
  }());
```





# 2.非Web版编译运行
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