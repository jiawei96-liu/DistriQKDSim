#include "RouteFactory.h"
#include "BfsStrategy.h"
#include "KeyRateStrategy.h"
#include "BidBfsStrategy.h"
#include "KeyRateFibStrategy.h"
#include "demoRoute.h"
#include "Alg/Network.h"
#include <iostream>

// #include "YourStrategy.h" //自定义路由函数文件

using namespace route ;

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
    }else if(type==RouteType_demo){
        cout << "自定义路由算法" << endl;
        return std::make_unique<BfsStrategy>(net);  //自定义路由算法1
    
    }else {
        std::cerr << "Unknown strategy type: " << type << std::endl;
    }
    return nullptr;
}

