#include "RouteFactory.h"
#include "BfsStrategy.h"
#include "Network.h"
#include <iostream>

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
        return std::make_unique<BfsStrategy>(net);
    } else {
        std::cerr << "Unknown strategy type: " << type << std::endl;
    }
    return nullptr;
}

