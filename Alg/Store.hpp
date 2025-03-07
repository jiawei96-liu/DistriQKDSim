#pragma once
#include "stdafx.h"

class SimResultStatus
{
private:
    /* data */
public:
    SimResultStatus(/* args */)=default;
    ~SimResultStatus()=default;
    
    DEMANDID demandId;
    NODEID nodeId;
    NODEID nextNode;
    LINKID minLink;
    VOLUME availableKeys;
    VOLUME remainVolume;
    bool isDelivered;
    bool isWait;
    bool isRouteFailed;
    TIME completeTime;
};

class SimResultStore
{
private:
    /* data */
public:
    SimResultStore(/* args */)=default;
    ~SimResultStore()=default;
    vector<vector<SimResultStatus>> store;
};



