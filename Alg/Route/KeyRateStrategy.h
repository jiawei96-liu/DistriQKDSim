#ifndef KEYRATESTRATEGY_H
#define KEYRATESTRATEGY_H

#include "RouteFactory.h"
namespace route {

class KeyRateStrategy : public RouteStrategy {
private:
    CNetwork *net;
public:
    KeyRateStrategy(CNetwork* _net){
        net=_net;
    }
    ~KeyRateStrategy()  = default;

    bool Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) override;
};

} // namespace route

#endif // KEYRATESTRATEGY_H
