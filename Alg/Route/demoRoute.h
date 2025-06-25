#ifndef BFS_STRATEGY_H
#define BFS_STRATEGY_H

#include "RouteFactory.h"

namespace route {

class BfsStrategy : public RouteStrategy {
private:
    CNetwork *net;
public:
    BfsStrategy(CNetwork* _net){
        net=_net;
    }
    ~BfsStrategy()  = default;

    bool Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) override;
};

} // namespace route

#endif // BFS_STRATEGY_H
