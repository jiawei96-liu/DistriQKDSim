#ifndef KEYRATEFIBSTRATEGY_H
#define KEYRATEFIBSTRATEGY_H

#include "RouteFactory.h"
namespace route {

class KeyRateFibStrategy : public RouteStrategy {
private:
    CNetwork *net;
public:
    KeyRateFibStrategy(CNetwork* _net){
        net=_net;
    }
    ~KeyRateFibStrategy()  = default;

    bool Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) override;
};

} // namespace route
#endif // KEYRATEFIBSTRATEGY_H
