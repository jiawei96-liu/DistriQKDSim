#ifndef SCHED_FACTORY_H
#define SCHED_FACTORY_H

#include<memory>
#include "Alg/stdafx.h"

class CNetwork;

namespace sched
{

typedef enum {
    SchedType_Min,
    SchedType_Avg,
    SchedType_Custom,
    SchedType_Unknown
} SchedStrategyType;

class SchedStrategy
{
private:
    /* data */
public:
    SchedStrategy(/* args */)=default;
    ~SchedStrategy()=default;
    virtual TIME Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) = 0;
};  

class SchedFactory
{
private:
    /* data */
    CNetwork * net;
public:
    SchedFactory(CNetwork * net);
    ~SchedFactory();
    std::unique_ptr<SchedStrategy> CreateStrategy(SchedStrategyType type);
};


} // namespace sched

#endif
