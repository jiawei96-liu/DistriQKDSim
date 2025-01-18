#include "QKDSim.h"
#include "Network.h"
#include "stdafx.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>  // 用于捕获异常
#include <cctype>  // 用于检查字符

using namespace std;

// 清理字符串中的不可见字符，如 BOM 和空格
std::string cleanString(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (std::isprint(c) || std::isspace(c)) {  // 仅保留可打印字符和空格
            result += c;
        }
    }
    return result;
}


// 打印每个字符的ASCII值，用于调试
void printAscii(const std::string& str) {
    for (char c : str) {
        std::cout << "Character: '" << c << "' ASCII: " << (int)c << std::endl;
    }
}

// 判断字符串是否是有效的整数
bool isValidInteger(const std::string& str) {
    for (char c : str) {
        if (!isdigit(c) && c != '-') {  // 允许负号
            return false;
        }
    }
    return true;
}

// 加载CSV文件中的数据
void QKDSim::loadCSV(const std::string& fileName, Kind kind) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return;
    }

    std::string line;
    if (kind == Network) {
        network.clear();

        // 读取链路数量（第一行）
        int NodeNum = 0;
        if (std::getline(file, line)) {
            std::istringstream(line) >> NodeNum;  // 读取链路数量
        }
        cout << "Node Number: " << NodeNum << endl;

        // 逐行读取链路信息
        while (std::getline(file, line)) {
            std::istringstream stream(line);
            std::string field;
            std::vector<std::string> fields;
            while (std::getline(stream, field, ',')) {
                fields.push_back(field);  // 清理每个字段的空格
            }

            // 如果字段少于8个，则跳过此行
            if (fields.size() < 8) {
                continue;
            }

            // 检查每个字段的有效性
            try {
                int field1 = std::stoi(fields[0]);  // 链路ID
                int field2 = std::stoi(fields[1]);  // 源节点ID
                int field3 = std::stoi(fields[2]);  // 目标节点ID
                double field4 = std::stod(fields[3]);  // QKD速率
                double field5 = std::stod(fields[4]);  // 延迟
                double field6 = std::stod(fields[5]);  // 带宽
                double field7 = std::stod(fields[6]);  // 权重
                double field8 = (fields[7].empty() || fields[7] == "-1") ? -1 : std::stod(fields[7]);  // 故障时间

                // 将链路数据存储到network中
                network.push_back(std::make_tuple(field1, field2, field3, field4, field5, field6, field7, field8));
            } catch (const std::invalid_argument& e) {
                // 如果有无效数据，跳过这一行并输出错误信息
                std::cerr << "Invalid data in line: " << line << std::endl;
                continue;
            } catch (const std::out_of_range& e) {
                // 如果数据超出范围，跳过这一行并输出错误信息
                std::cerr << "Data out of range in line: " << line << std::endl;
                continue;
            }
        }
    } else if (kind == Demand) {
        demand.clear();  // 清空旧的需求数据
        while (std::getline(file, line)) {  // 逐行读取文件内容
            // std::cout << "Reading line: " << line << std::endl;

            // 清理行内容
            line = cleanString(line);
            // std::cout << "Cleaned line: " << line << std::endl;

            std::istringstream stream(line);  // 创建输入字符串流对象，用于按行处理数据
            std::string field;
            std::vector<std::string> fields;  // 存储当前行分割后的各字段

            // 使用 ',' 作为分隔符，将当前行切分成各个字段
            while (std::getline(stream, field, ',')) {
                fields.push_back(field);  // 清理并存储每个字段
            }

            // // 打印出字段信息
            // std::cout << "Reading line: " << line << std::endl;
            // std::cout << "Number of fields: " << fields.size() << std::endl;
            // for (size_t i = 0; i < fields.size(); ++i) {
            //     std::cout << "Field " << i << ": " << fields[i] << std::endl;
            // }

            // 解析需求数据，如果字段少于5个，说明这一行数据不完整，跳过
            if (fields.size() < 5) {
                continue;
            }

            try {
                // // 确保每个字段是有效的整数或浮点数
                // if (!isValidInteger(fields[0])) {
                //     std::cerr << "Invalid integer field: " << fields[0] << " in line: " << line << std::endl;
                //     continue;
                // }

                int field1 = std::stoi(fields[0]);  // 需求ID
                int field2 = std::stoi(fields[1]);  // 源节点ID
                int field3 = std::stoi(fields[2]);  // 目标节点ID
                double field4 = std::stod(fields[3]);  // 流量
                double field5 = std::stod(fields[4]);  // 到达时间

                // 将需求数据存储到demand中
                demand.push_back(std::make_tuple(field1, field2, field3, field4, field5));
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid demand data in line: " << line << " Error: " << e.what() << std::endl;
                continue;
            } catch (const std::out_of_range& e) {
                std::cerr << "Demand data out of range in line: " << line << " Error: " << e.what() << std::endl;
                continue;
            }
        }
    }
}




// 将加载的数据用于初始化CNetwork
void QKDSim::readCSV(Kind kind) {
    if (kind == Network) {
        // 假设从文件或其他方式获取到的节点数（可以根据实际情况调整）
        int nodeNum = 20000;  // 这里假设节点数是1w
        net->SetNodeNum(nodeNum);
        net->InitNodes(nodeNum);

        // std::cout << "Initializing Network with " << nodeNum << " nodes." << std::endl;

        // 逐行读取链路信息
        for (const auto& netw : network) {
            int linkId = std::get<0>(netw);
            int sourceId = std::get<1>(netw);
            int sinkId = std::get<2>(netw);
            double keyRate = std::get<3>(netw);
            double proDelay = std::get<4>(netw);
            double bandWidth = std::get<5>(netw);
            double weight = std::get<6>(netw);
            double faultTime = std::get<7>(netw);

            // 打印链路信息
            std::cout << "Processing Link: " << linkId << std::endl;
            std::cout << "  Source ID: " << sourceId << ", Sink ID: " << sinkId << std::endl;
            std::cout << "  Key Rate: " << keyRate << ", Delay: " << proDelay << ", Bandwidth: " << bandWidth << std::endl;
            std::cout << "  Weight: " << weight << ", Fault Time: " << faultTime << std::endl;

            // 处理链路信息
            CLink newLink;
            newLink.SetLinkId(linkId);
            newLink.SetSourceId(sourceId);
            newLink.SetSinkId(sinkId);
            newLink.SetQKDRate(keyRate);
            newLink.SetLinkDelay(proDelay);
            newLink.SetBandwidth(bandWidth);
            newLink.SetWeight(weight);
            newLink.SetFaultTime(faultTime);

            // 将链路添加到网络
            net->m_vAllLinks.push_back(newLink);
            net->m_mNodePairToLink[make_pair(sourceId, sinkId)] = linkId;
            net->m_mNodePairToLink[make_pair(sinkId, sourceId)] = linkId;
            net->InitKeyManagerOverLink(linkId);

            // 打印相邻节点信息
            std::cout << "  Adding " << sinkId << " as adjacent to " << sourceId << std::endl;
            net->m_vAllNodes[sourceId].m_lAdjNodes.push_back(sinkId);
            net->m_vAllNodes[sinkId].m_lAdjNodes.push_back(sourceId);

            // 假设这部分用于处理故障时间
            if (faultTime > 0) {
                net->m_mDemandArriveTime.insert(make_pair(faultTime, 1000000 + linkId));
                std::cout << "  Fault time detected. Inserting into m_mDemandArriveTime." << std::endl;
            }
        }

        // 设置链路数量
        net->SetLinkNum(static_cast<int>(network.size()));
        std::cout << "Total links processed: " << network.size() << std::endl;
    } else if (kind == Demand) {
        // 处理需求数据
        std::cout << "Processing Demand Data..." << std::endl;
        for (const auto& dem : demand) {
            // int demandId = std::get<0>(dem);
            // int sourceId = std::get<1>(dem);
            // int sinkId = std::get<2>(dem);
            // double flow = std::get<3>(dem);
            // double arriveTime = std::get<4>(dem);

            DEMANDID demandId = std::get<0>(dem);
            NODEID sourceId = std::get<1>(dem);
            NODEID sinkId = std::get<2>(dem);
            VOLUME demandVolume = std::get<3>(dem);
            TIME arriveTime = std::get<4>(dem);

            // 打印需求信息
            std::cout << "Processing Demand: " << demandId << std::endl;
            std::cout << "  Source ID: " << sourceId << ", Sink ID: " << sinkId << std::endl;
            std::cout << "  demandVolume: " << demandVolume << ", Arrive Time: " << arriveTime << std::endl;

            // 处理需求信息
            CDemand newDemand;
            newDemand.SetDemandId(demandId);
            newDemand.SetSourceId(sourceId);
            newDemand.SetSinkId(sinkId);
            newDemand.SetDemandVolume(demandVolume);
            newDemand.SetRemainingVolume(demandVolume);
            newDemand.SetArriveTime(arriveTime);
            newDemand.SetCompleteTime(INF); // 假设 INF 是一个定义好的常量，表示无限大
            net->m_vAllDemands.push_back(newDemand);
            net->m_vAllNodes[sourceId].m_mRelayVolume[demandId] = demandVolume;   //对m_mRelayVolume做初始化
            net->m_mDemandArriveTime.insert(make_pair(arriveTime, demandId));   // 增加m_mDemandArriveTime
        }

        net->SetDemandNum(static_cast<int>(demand.size()));

        }

        std::cout << "Total demands processed: " << demand.size() << std::endl;
    }


