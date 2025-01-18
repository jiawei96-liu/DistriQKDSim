#include "QKDSim.h"
#include "./Network.h"  // 包含 CNetwork 类的头文件
#include <chrono>       // 用于计时

int main() {
    // 假设您已经有了 CNetwork 的实现
    CNetwork network;
    QKDSim sim(&network);

    // 加载网络数据
    // sim.QKDSim::loadCSV("../Input/10规模/network.csv", Network);
    // sim.QKDSim::loadCSV("../Input/network.csv", Network);
    sim.QKDSim::loadCSV("../Input/network(10000).csv", Network);
    sim.readCSV(Network);

    // 加载需求数据
    // sim.QKDSim::loadCSV("../Input/10规模/demand.csv", Demand);
    // sim.QKDSim::loadCSV("../Input/demand.csv", Demand);
    sim.QKDSim::loadCSV("../Input/demand(10000).csv", Demand);
    sim.readCSV(Demand);

    // 统计 InitRelayPath 函数的执行时间
    auto start = std::chrono::high_resolution_clock::now();
    network.InitRelayPath();
    auto end_1 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_1 = end_1 - start;
    std::cout << "路径初始化耗时: " << elapsed_1.count() << " s." << std::endl;


    network.ShowDemandPaths();
    while (!network.AllDemandsDelivered())
    {
        TIME executeTime = network.OneTimeRelay();
        cout << "进行onetimerelay" << endl;
        network.MoveSimTime(executeTime);
        cout << "推进执行时间" << endl;
    }
    std::cout << "全部需求传输完毕: " << std::endl;
    
    auto end_2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_2 = end_2 - start;
    std::cout << "总耗时: " << elapsed_2.count() << " s." << std::endl;

    return 0;
}

