#pragma once
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <vector>

#include "Web/dto/DTOs.hpp"
#include "Web/dto/ListDTO.hpp"
#include "Alg/Store.hpp"

class SimDao
{
private:
    /* data */
public:
    SimDao(const std::string& host, const std::string& user, const std::string& password, const std::string& database);
    ~SimDao();

    bool getAllSimRes(PageDto<SimResDto>& res,uint32_t offset,uint32_t limit);
    int batchInsertSimulationResults(int simId,int step,double currentTime,vector<SimResultStatus> &simRes); 
    int getOrCreateStepID(int simId, int step, double currentTime);
    int deleteSimulations(int simId);
    int getSimRes(int simId, int step, vector<SimResDto>& res,TIME& currentTime);
    int getSimStatus(int simId,SimStatusDto& status);
    int setSimStatus(int simId,string status);
    int setSimStepAndTime(int simId,int step,int currentTime);
    int createSim(int simId,string name,string routeAlg,string scheduleAlg);
    int clear();
private:
    sql::mysql::MySQL_Driver* driver;
    std::unique_ptr<sql::Connection> con;
    std::string host;
    std::string user;
    std::string password;
    std::string database;
};

