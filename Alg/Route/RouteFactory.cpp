#include "RouteFactory.h"
#include "BfsStrategy.h"
#include "KeyRateStrategy.h"
#include "BidBfsStrategy.h"
#include "KeyRateFibStrategy.h"
#include "CustomRouteStrategy.h"
#include "OspfStrategy.h"
#include "BgpStrategy.h"
#include "MaxMinRateStrategy.h"
#include "MinTimeStrategy.h"
#include <dlfcn.h>
#include <memory>
#include "Alg/Network.h"
#include <iostream>

// #include "YourStrategy.h" //自定义路由函数文件

using namespace route ;

// // 函数指针类型：用户动态库中导出的 createRouteStrategy 工厂函数
using CreateFunc = CustomRouteStrategy* (*)(CNetwork*);

namespace {
    void* handle = nullptr;
    CreateFunc createFunc = nullptr;
}
std::unique_ptr<CustomRouteStrategy> loadUserCustomRouteStrategy(CNetwork* net) {
    if (!handle) {
        handle = dlopen("../Alg/Route/libUserRoute.so", RTLD_NOW);
        if (!handle) {
            std::cerr << "dlopen failed: " << dlerror() << std::endl;
            return nullptr;
        }

        createFunc = (CreateFunc)dlsym(handle, "createRouteStrategy");
        if (!createFunc) {
            std::cerr << "dlsym failed: " << dlerror() << std::endl;
            dlclose(handle);
            handle = nullptr;
            return nullptr;
        }
    }

    return std::unique_ptr<CustomRouteStrategy>(createFunc(net));
}

void unloadUserRouteStrategy() {
    if (handle) {
        dlclose(handle);
        handle = nullptr;
        createFunc = nullptr;
        std::cout << "旧的自定义策略库已卸载" << std::endl;
    }
}

RouteFactory::RouteFactory(CNetwork* network) : net(network) {
    // std::cout << "RouteFactory created with network\n";
}

RouteFactory::~RouteFactory() {
    // std::cout << "RouteFactory destroyed\n";
}

std::unique_ptr<RouteStrategy> RouteFactory::CreateStrategy(RouteStrategyType type) {
    if (type == RouteType_Bfs) {
        // std::cout << "Creating BFS Strategy\n";
        cout << "BFS" << endl;
        return std::make_unique<BfsStrategy>(net);  //朴素BFS
        // return std::make_unique<BidBfsStrategy>(net);    //双向BFS
    }else if(type==RouteType_KeyRateShortestPath){
        cout << "密钥速率Dijkstra" << endl;
        return std::make_unique<KeyRateStrategy>(net);  //二叉堆
        
        // return std::make_unique<KeyRateFibStrategy>(net);    //斐波那契堆
    } else if (type == RouteType_Ospf) {
        cout << "OSPF" << endl;
        return std::make_unique<OspfStrategy>(net);
    } else if (type == RouteType_Bgp) {
        cout << "BGP" << endl;
        return std::make_unique<BgpStrategy>(net);
    } else if (type == RouteType_Custom) {
        cout << "自定义路由算法" << endl;
        unloadUserRouteStrategy();
        return loadUserCustomRouteStrategy(net);  //动态加载用户实现
    } else if (type == RouteType_MaxMinRate) {
        cout << "最大最小速率策略" << endl;
        return std::make_unique<MaxMinRateStrategy>(net);
    } else if (type == RouteType_MinTime) {
        cout << "最小时间策略" << endl;
        return std::make_unique<MinTimeStrategy>(net);
    } else {
        std::cerr << "Unknown strategy type: " << type << std::endl;
    }
    return nullptr;
}



std::unique_ptr<RouteStrategy> RouteFactory::CreateUserStrategy(std::string soPath) {
    if (soPath==""){
        cout<<"[Error]CreateUserStrategy:No soPath Found"<<endl;
        return nullptr;
    }

    void* handler = nullptr;
    CreateFunc creater = nullptr;
    if (!handler) {
        std::cout<<"dlopen "+soPath<<endl;
        handler = dlopen(soPath.c_str(), RTLD_NOW);
        if (!handler) {
            std::cerr << "dlopen failed: " << dlerror() << std::endl;
            return nullptr;
        }

        creater = (CreateFunc)dlsym(handler, "createRouteStrategy");
        if (!creater) {
            std::cerr << "dlsym failed: " << dlerror() << std::endl;
            dlclose(handler);
            handler = nullptr;
            return nullptr;
        }
    }

    return std::unique_ptr<CustomRouteStrategy>(creater(net));
}