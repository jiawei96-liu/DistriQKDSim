#ifndef RESERVATION_STRATEGY_H
#define RESERVATION_STRATEGY_H

#include "SchedFactory.h"

namespace sched {

class ReservationStrategy : public SchedStrategy {
private:
    CNetwork *net;
public:
    ReservationStrategy(CNetwork* _net){ net=_net; }
    ~ReservationStrategy() = default;

    TIME Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) override;
};

} // namespace sched

#endif // RESERVATION_STRATEGY_H
