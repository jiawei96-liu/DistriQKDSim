#ifndef BIDBFSSTRATEGY_H
#define BIDBFSSTRATEGY_H
#include "RouteFactory.h"


namespace route {

class BidBfsStrategy : public RouteStrategy {
private:
    CNetwork *net;
public:
    BidBfsStrategy(CNetwork* _net){
        net=_net;
    }
    ~BidBfsStrategy()  = default;

    bool Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList) override;
private:
    bool ExpandFrontier(queue<NODEID>& frontierQueue, vector<bool>& visitedThisSide,
                                        vector<bool>& visitedOtherSide, vector<NODEID>& preNode,
                                        NODEID& meetingNode, bool isSourceSide);

    void BuildPath(NODEID sourceId, NODEID sinkId, NODEID meetingNode,
                                   vector<NODEID>& preNodeSource, vector<NODEID>& preNodeSink,
                                   std::list<NODEID>& nodeList, std::list<LINKID>& linkList);
};

} // namespace route

#endif // BIDBFSSTRATEGY_H
