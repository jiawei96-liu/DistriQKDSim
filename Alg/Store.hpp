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



class SimMetric
{
public:
    int step = 0;
    double CurrentTime = 0.0;
    double TransferredVolume = 0.0;
    double TransferredPercent = 0.0;
    double RemainingVolume = 0.0;
    double TransferRate = 0.0;
    int InProgressDemandCount = 0;
};
