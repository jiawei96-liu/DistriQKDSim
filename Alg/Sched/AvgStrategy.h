#ifndef AVG_STRATEGY_H
#define AVG_STRATEGY_H

#include "SchedFactory.h"

namespace sched {

class AvgStrategy : public SchedStrategy {
private:
    CNetwork *net;
public:
    AvgStrategy(CNetwork* _net){
        net=_net;
    }
    ~AvgStrategy()  = default;

    TIME Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) override;
};

} // namespace route

#endif // AVG_STRATEGY_H
