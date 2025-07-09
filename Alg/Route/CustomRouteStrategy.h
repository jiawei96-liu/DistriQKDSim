#ifndef CUSTOM_STRATEGY_H
#define CUSTOM_STRATEGY_H

#include "RouteFactory.h"

namespace route {

class CustomRouteStrategy : public RouteStrategy {
private:
    CNetwork *net;
public:
    CustomRouteStrategy(CNetwork* _net){
        net=_net;
    }
    ~CustomRouteStrategy()  = default;

    bool Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) override;
};

} // namespace route

#endif // BFS_STRATEGY_H