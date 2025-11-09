#ifndef SCHED_FACTORY_H
#define SCHED_FACTORY_H

#include<memory>
#include "Alg/stdafx.h"

class CNetwork;

namespace sched
{

/**
 * 调度策略类型枚举
 * 定义系统支持的所有调度策略类型
 */
typedef enum {
    SchedType_Min,       // 最小任务完成优先算法
    SchedType_Avg,       // 平均密钥调度算法
    SchedType_Reserve,   // 端到端预约调度算法
    SchedType_FCFS,      // 先到先服务调度算法
    SchedType_Custom,    // 自定义调度算法
    SchedType_Unknown    // 未知调度算法
} SchedStrategyType;

/**
 * 调度策略抽象基类
 * 所有调度策略都必须继承此类并实现Sched方法
 */
class SchedStrategy
{
private:
    /* data */
public:
    /**
     * 默认构造函数
     */
    SchedStrategy(/* args */)=default;
    
    /**
     * 虚析构函数
     */
    ~SchedStrategy()=default;
    
    /**
     * 调度算法接口
     * 所有派生类都必须实现此方法
     * 
     * @param nodeId 当前要调度的节点ID
     * @param relayDemands 输出参数，存储调度结果（需求ID -> 分配的数据量）
     * @return TIME 返回估计的执行时间，如果没有可执行的任务返回INF
     */
    virtual TIME Sched(NODEID nodeId, map<DEMANDID, VOLUME> &relayDemands) = 0;
};  

/**
 * 调度策略工厂类
 * 负责根据策略类型创建对应的调度策略实例
 */
class SchedFactory
{
private:
    /* data */
    CNetwork * net;  // 网络实例指针

public:
    /**
     * 构造函数
     * @param net 网络实例指针
     */
    SchedFactory(CNetwork * net);
    
    /**
     * 析构函数
     */
    ~SchedFactory();
    
    /**
     * 策略创建方法
     * 根据策略类型创建对应的调度策略实例
     * 
     * @param type 调度策略类型
     * @return std::unique_ptr<SchedStrategy> 调度策略实例指针
     */
    std::unique_ptr<SchedStrategy> CreateStrategy(SchedStrategyType type);
};


} // namespace sched

#endif