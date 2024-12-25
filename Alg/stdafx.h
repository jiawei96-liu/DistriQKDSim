// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
//#include <tchar.h>



// TODO: reference additional headers your program requires here
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <set>
#include <utility>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
// #include <Windows.h>
#include <algorithm>
#include <time.h>
class CNetwork;
using namespace std;

typedef unsigned int UINT;

typedef UINT NODEID;
typedef UINT LINKID;
typedef double TIME;
typedef double VOLUME;
typedef double RATE;
typedef UINT EVENTID;
typedef UINT DEMANDID;
typedef UINT KEYID;

typedef double WEIGHT;
typedef bool (CNetwork::*CalcuPath)(NODEID, NODEID, list<NODEID>&, list<LINKID>&);

const UINT IMPOSSIBLENODE = -1;
const double THRESHOLD = 0.01;
const double SMALLNUM = 0.0001;
const double INFSMALL = 0.000001;
const EVENTID IMPOSSIBLEEVENT = -1;
const UINT INFUINT = 100000000;
const double INF = 10000000000.0;
const double LARGEVALUE = 100000.0;
const TIME IMPOSSIBLETIME = -0.1;
const double NEGSMALLVALUE = -0.0001;

enum EVENTTYPE {DEMANDARRIVE, DEMANDCOMPLETE};
