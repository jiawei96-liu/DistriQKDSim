#ifndef MIN_STRATEGY_H
#define MIN_STRATEGY_H

#include "SchedFactory.h"

namespace sched {

class MinStrategy : public SchedStrategy {
private:
    CNetwork *net;
public:
    MinStrategy(CNetwork* _net){
        net=_net;
    }
    ~MinStrategy()  = default;

    TIME Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) override;
};

} // namespace route

#endif // MIN_STRATEGY_H
