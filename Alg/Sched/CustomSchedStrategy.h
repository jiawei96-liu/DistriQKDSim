#ifndef CUSTOM_SCHED_STRATEGY_H
#define CUSTOM_SCHED_STRATEGY_H

#include "SchedFactory.h"

namespace sched {

class CustomSchedStrategy : public SchedStrategy {
private:
    CNetwork *net;
public:
    CustomSchedStrategy(CNetwork* _net){
        net=_net;
    }
    ~CustomSchedStrategy()  = default;

    TIME Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) override;
};

} // namespace sched

#endif // BFS_STRATEGY_H