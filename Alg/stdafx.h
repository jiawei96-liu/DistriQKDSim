// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

// 判断操作系统
#ifdef _WIN32
    // Windows 系统相关的代码
    #include <SDKDDKVer.h>  // Windows SDK 相关
    #include <Windows.h>    // Windows 特定的头文件
#elif __linux__
    // Linux 系统相关的代码
    #include <iostream>
    #include <fstream>
    #include <sstream>
    #include <vector>
    #include <list>
    #include <map>
    #include <set>
    #include <utility>
    #include <algorithm>
    #include <ctime>  // Linux 下使用 time.h 替代 Windows.h 中的时间相关功能
    #include <unistd.h>  // 用于文件操作、进程控制等
    #include <pthread.h>  // 用于线程相关功能
    #include <queue>
    #include <utility>
    #include <string>
    #include <unordered_map>
    #include <unordered_set>
    #include <time.h>
#endif

// CNetwork 的前向声明
class CNetwork;

using namespace std;

// 类型别名定义
typedef unsigned int UINT;  // 定义 UINT 类型
typedef UINT NODEID;        // 节点 ID 类型
typedef UINT LINKID;        // 链路 ID 类型
typedef double TIME;        // 时间类型
typedef double VOLUME;      // 流量类型
typedef double RATE;        // 速率类型
typedef UINT EVENTID;       // 事件 ID 类型
typedef UINT DEMANDID;      // 需求 ID 类型
typedef UINT KEYID;         // 密钥 ID 类型
typedef double WEIGHT;      // 权重类型

// 常量定义
const double INFSMALL = 0.000001;
const double SMALLNUM = 0.0001;
const double INF = 10000000000.0;
const double LARGEVALUE = 100000.0;
const double NEGSMALLVALUE = -0.0001;
const UINT INFUINT = 100000000;
const TIME IMPOSSIBLETIME = -0.1;
const UINT IMPOSSIBLENODE = -1;
const EVENTID IMPOSSIBLEEVENT = -1;
const double THRESHOLD = 0.01;
const double KEY_RATE_TO_NEGOTIATION_RATIO = 0.1;  // 密钥速率与协商数据速率的比例

// 函数指针类型
typedef bool (CNetwork::*CalcuPath)(NODEID, NODEID, std::list<NODEID>&, std::list<LINKID>&);

// 枚举定义
enum EVENTTYPE { DEMANDARRIVE, DEMANDCOMPLETE };






