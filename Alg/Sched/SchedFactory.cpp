#include "SchedFactory.h"
#include "MinStrategy.h"
#include "AvgStrategy.h"
#include "ReservationStrategy.h"
#include "FCFSStrategy.h"
#include "CustomSchedStrategy.h"
#include "Alg/Network.h"
#include <dlfcn.h>
#include <memory>
#include <iostream>

using namespace sched ;

// 函数指针类型：用户动态库中导出的 createSchedStrategy 工厂函数
using CreateFunc = CustomSchedStrategy* (*)(CNetwork*);

namespace {
    void* handle = nullptr;
    CreateFunc createFunc = nullptr;
}

/**
 * 加载用户自定义调度策略
 * 动态加载用户实现的调度策略库
 * 
 * @param net 网络实例指针
 * @return std::unique_ptr<CustomSchedStrategy> 用户自定义调度策略实例指针
 */
std::unique_ptr<CustomSchedStrategy> loadUserCustomSchedStrategy(CNetwork* net) {
    if (!handle) {
        handle = dlopen("../Alg/Sched/libUserSched.so", RTLD_NOW);
        if (!handle) {
            std::cerr << "dlopen failed: " << dlerror() << std::endl;
            return nullptr;
        }

        createFunc = (CreateFunc)dlsym(handle, "createSchedStrategy");
        if (!createFunc) {
            std::cerr << "dlsym failed: " << dlerror() << std::endl;
            dlclose(handle);
            handle = nullptr;
            return nullptr;
        }
    }

    return std::unique_ptr<CustomSchedStrategy>(createFunc(net));
}

/**
 * 卸载用户自定义调度策略
 * 释放已加载的用户调度策略库
 */
void unloadUserSchedStrategy() {
    if (handle) {
        dlclose(handle);
        handle = nullptr;
        createFunc = nullptr;
        std::cout << "旧的自定义策略库已卸载" << std::endl;
    }
}

/**
 * 构造函数
 * @param network 网络实例指针
 */
SchedFactory::SchedFactory(CNetwork* network) : net(network) {
    // std::cout << "SchedFactory created with network\n";
}

/**
 * 析构函数
 */
SchedFactory::~SchedFactory() {
    // std::cout << "SchedFactory destroyed\n";
}

/**
 * 策略创建方法
 * 根据策略类型创建对应的调度策略实例
 * 
 * @param type 调度策略类型
 * @return std::unique_ptr<SchedStrategy> 调度策略实例指针
 */
std::unique_ptr<SchedStrategy> SchedFactory::CreateStrategy(SchedStrategyType type) {
    if (type == SchedType_Min) {
        // std::cout << "Creating BFS Strategy\n";
        cout << "最小任务完成优先算法" << endl;
        return std::make_unique<MinStrategy>(net);  
    }else if(type==SchedType_Avg){
        cout << "平均密钥调度算法" << endl;
        return std::make_unique<AvgStrategy>(net);
    }else if(type==SchedType_Reserve){
        cout << "端到端预约调度算法" << endl;
        return std::make_unique<ReservationStrategy>(net);
    }
    else if(type==SchedType_FCFS){
        cout << "先到先服务调度算法" << endl;
        return std::make_unique<FCFSStrategy>(net);
    }
    else if(type==SchedType_Custom){
        cout << "自定义调度算法" << endl;
        unloadUserSchedStrategy();
        return loadUserCustomSchedStrategy(net);  //动态加载用户实现
    }
    else {
        std::cerr << "Unknown strategy type: " << type << std::endl;
    }
    return nullptr;
}