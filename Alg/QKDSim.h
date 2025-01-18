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
    QKDSim(CNetwork* net) : net(net) {}

    void loadCSV(const std::string& fileName, Kind kind);
    void readCSV(Kind kind);

private:
    std::vector<std::tuple<int, int, int, double, double, double, double, double>> network;
    std::vector<std::tuple<int, int, int, double, double>> demand;
    CNetwork* net;  // CNetwork类的指针
};

#endif  // QKDSIM_H
