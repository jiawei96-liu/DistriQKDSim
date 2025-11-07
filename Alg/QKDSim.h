#ifndef QKDSIM_H
#define QKDSIM_H

#include <vector>
#include <tuple>
#include <string>
#include <map>

// 前向声明CLink和CNetwork类
class CLink;
class CNetwork;

enum Kind { Network, Demand };

class QKDSim {
public:
    QKDSim(CNetwork* net) : net(net) {
        network=std::vector<std::tuple<int, int, int, double, double, double, double, double>>();
        demand=std::vector<std::tuple<int, int, int, double, double>>();
    }

    void loadCSV(const std::string& fileName, Kind kind);
    void readCSV(Kind kind);
    CNetwork* net;  // CNetwork类的指针

private:
    std::vector<std::tuple<int, int, int, double, double, double, double, double>> network;
    std::vector<std::tuple<int, int, int, double, double>> demand;
};

#endif  // QKDSIM_H
