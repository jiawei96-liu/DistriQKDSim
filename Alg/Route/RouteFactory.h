#ifndef ROUTE_FACTORY_H
#define ROUTE_FACTORY_H

#include<memory>
#include "Alg/StdAfx.h"

class CNetwork;

namespace route
{

typedef enum {
    RouteType_Bfs,
    RouteType_KeyRateShortestPath,
    RouteType_Unknown
} RouteStrategyType;

class RouteStrategy
{
private:
    /* data */
public:
    RouteStrategy(/* args */)=default;
    ~RouteStrategy()=default;
    virtual bool Route(NODEID sourceId, NODEID sinkId, list<NODEID>& nodeList, list<LINKID>& linkList) =0;
};  

class RouteFactory
{
private:
    /* data */
    CNetwork * net;
public:
    RouteFactory(CNetwork * net);
    ~RouteFactory();
    std::unique_ptr<RouteStrategy> CreateStrategy(RouteStrategyType type);
};


} // namespace route

#endif
