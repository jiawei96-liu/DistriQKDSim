#ifndef MIN_TIME_STRATEGY_H
#define MIN_TIME_STRATEGY_H

#include "RouteFactory.h"

namespace route {

class MinTimeStrategy : public RouteStrategy {
private:
    CNetwork *net;
    
    // 计算单个链路的传输时间
    // 参数: linkId - 链路ID
    // 参数: dataAmount - 需要传输的总数据量
    // 返回: 在该链路上的传输时间
    double CalculateLinkTime(LINKID linkId, double dataAmount);
    
public:
    MinTimeStrategy(CNetwork* _net) {
        net = _net;
    }
    ~MinTimeStrategy() = default;

    // 带数据量参数的Route函数
    bool Route(NODEID sourceId, NODEID sinkId, std::list<NODEID>& nodeList, std::list<LINKID>& linkList, double dataAmount);
};

} // namespace route

#endif // MIN_TIME_STRATEGY_H
