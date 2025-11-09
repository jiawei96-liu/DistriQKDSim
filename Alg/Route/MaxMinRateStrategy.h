#ifndef MAXMINRATESTRATEGY_H
#define MAXMINRATESTRATEGY_H

#include "RouteFactory.h"

namespace route {

class MaxMinRateStrategy : public RouteStrategy {
private:
    CNetwork *net;
public:
    MaxMinRateStrategy(CNetwork* _net){
        net=_net;
    }
    ~MaxMinRateStrategy()  = default;

    bool Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) override;
};

} // namespace route

#endif // MAXMINRATESTRATEGY_H