#include "SchedFactory.h"
#include "MinStrategy.h"
#include "AvgStrategy.h"
#include "CustomSchedStrategy.h"
#include "Alg/Network.h"
#include <dlfcn.h>
#include <memory>
#include <iostream>

using namespace sched ;

// // 函数指针类型：用户动态库中导出的 createRouteStrategy 工厂函数
using CreateFunc = CustomSchedStrategy* (*)(CNetwork*);

namespace {
    void* handle = nullptr;
    CreateFunc createFunc = nullptr;
}
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

void unloadUserSchedStrategy() {
    if (handle) {
        dlclose(handle);
        handle = nullptr;
        createFunc = nullptr;
        std::cout << "旧的自定义策略库已卸载" << std::endl;
    }
}

SchedFactory::SchedFactory(CNetwork* network) : net(network) {
    // std::cout << "SchedFactory created with network\n";
}

SchedFactory::~SchedFactory() {
    // std::cout << "SchedFactory destroyed\n";
}

std::unique_ptr<SchedStrategy> SchedFactory::CreateStrategy(SchedStrategyType type) {
    if (type == SchedType_Min) {
        // std::cout << "Creating BFS Strategy\n";
        cout << "最小任务完成优先算法" << endl;
        return std::make_unique<MinStrategy>(net);  
    }else if(type==SchedType_Avg){
        cout << "平均密钥调度算法" << endl;
        return std::make_unique<AvgStrategy>(net);
    }
    else if(type==SchedType_Custom){
        cout << "自定义路由算法" << endl;
        unloadUserSchedStrategy();
        return loadUserCustomSchedStrategy(net);  //动态加载用户实现
    }
    else {
        std::cerr << "Unknown strategy type: " << type << std::endl;
    }
    return nullptr;
}

