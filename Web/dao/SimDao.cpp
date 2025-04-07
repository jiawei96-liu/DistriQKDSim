#include"Web/dao/SimDao.hpp"
#include "Config/ConfigReader.h"
using namespace std;


SimDao::SimDao() {
    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con.reset(driver->connect(ConfigReader::getStr("db_host"),ConfigReader::getStr("user"), ConfigReader::getStr("password")));
        con->setSchema(ConfigReader::getStr("scheme")); // 选择数据库
    } catch (sql::SQLException& e) {
        std::cerr << "Database connection failed: " << e.what() << std::endl;
    }
}

SimDao::~SimDao(){

}

int SimDao::batchInsertSimulationResults(int simId,int step,double currentTime,vector<SimResultStatus> &simRes) {
    try {
        sql::PreparedStatement* pstmt;

        // **连接 MySQL**
        

        // **获取或创建 StepID**
        int stepID = getOrCreateStepID(simId, step, currentTime);
        if (stepID == -1) {
            cerr << "Failed to get or create StepID!" << endl;
            return -1;
        }

        // **开启事务**
        con->setAutoCommit(false);

        // **拼接 SQL 语句**
        string sql = "INSERT INTO SimulationResults (StepID, DemandID, NodeID, NextNodeID, NextHopLinkID, AvailableKeys, RemainVolume, Status, IsRouteFailed) VALUES ";

        for (size_t i = 0; i < simRes.size(); i++) {
            sql += "(" + to_string(stepID) + ", " + to_string(simRes[i].demandId) + ", " +
                   to_string(simRes[i].nodeId) + ", " + to_string(simRes[i].nextNode) + ", " +
                   to_string(simRes[i].minLink) + ", " + to_string(simRes[i].availableKeys) + ", " +
                   to_string(simRes[i].remainVolume) + ", '" + 
                   (simRes[i].isDelivered ? "DELIVERED" : (simRes[i].isWait ? "WAIT" : "IN_PROGRESS")) + "', '" +
                   (simRes[i].isRouteFailed ? "YES" : "NO") + "')";
            if (i < simRes.size() - 1) sql += ", ";
        }

        // **执行拼接好的 SQL 语句**
        auto stmt = con->createStatement();
        stmt->execute(sql);

        // **提交事务**
        con->commit();

        cout << "Batch insert completed!" << endl;

        // **释放资源**
        delete stmt;
        return 1;
    } catch (sql::SQLException &e) {
        cerr << "SQL Error (batchInsertSimulationResults): " << e.what() << endl;
        return -1;
    }
}

// **获取或创建 StepID**
int SimDao::getOrCreateStepID(int simId, int step, double currentTime) {
    try {
        // 查询 StepID
        auto pstmt = con->prepareStatement(
            "SELECT StepID FROM SimulationSteps WHERE SimID = ? AND Step = ?"
        );
        pstmt->setInt(1, simId);
        pstmt->setInt(2, step);
        auto res = pstmt->executeQuery();

        if (res->next()) {
            int stepID = res->getInt("StepID");
            delete res;
            delete pstmt;
            return stepID;
        }

        delete res;
        delete pstmt;

        // **Step 不存在，插入新 Step**
        pstmt = con->prepareStatement(
            "INSERT INTO SimulationSteps (SimID, Step, CurrentTime) VALUES (?, ?, ?)"
        );
        pstmt->setInt(1, simId);
        pstmt->setInt(2, step);
        pstmt->setDouble(3, currentTime);
        pstmt->executeUpdate();

        delete pstmt;

        // 获取刚插入的 StepID
        pstmt = con->prepareStatement("SELECT LAST_INSERT_ID()");
        res = pstmt->executeQuery();
        int newStepID = res->next() ? res->getInt(1) : -1;

        delete res;
        delete pstmt;
        return newStepID;
    } catch (sql::SQLException &e) {
        cerr << "SQL Error (getOrCreateStepID): " << e.what() << endl;
        return -1;
    }
}

int SimDao::deleteSimulations(int simId) {
    try {

        // 开始事务处理
        con->setAutoCommit(false);

        // 1. 删除 SimulationResults 表中的记录
        sql::PreparedStatement* pstmt1 = con->prepareStatement(
            "DELETE FROM SimulationResults WHERE StepID IN (SELECT StepID FROM SimulationSteps WHERE SimID = ?)"
        );
        pstmt1->setInt(1, simId);
        pstmt1->executeUpdate();
        delete pstmt1;

        // 2. 删除 SimulationSteps 表中的记录
        sql::PreparedStatement* pstmt2 = con->prepareStatement(
            "DELETE FROM SimulationSteps WHERE SimID = ?"
        );
        pstmt2->setInt(1, simId);
        pstmt2->executeUpdate();
        delete pstmt2;

        // 3. 删除 Simulations 表中的记录
        sql::PreparedStatement* pstmt3 = con->prepareStatement(
            "DELETE FROM Simulations WHERE SimID = ?"
        );
        pstmt3->setInt(1, simId);
        pstmt3->executeUpdate();
        delete pstmt3;

        // 提交事务
        con->commit();

        // 关闭连接

        cout << "Simulation " << simId << " and its associated data have been successfully deleted!" << endl;
        return 0;  // 删除成功
    } catch (sql::SQLException& e) {
        cerr << "Error during deletion: " << e.what() << endl;
        return -1;  // 删除失败
    }
}

int SimDao::getSimRes(int simId, int step, vector<SimResDto>& res,TIME& currentTime) {
    try {
        

        // 1. 获取 StepID
        sql::PreparedStatement* pstmt1 = con->prepareStatement(
            "SELECT StepID, CurrentTime FROM SimulationSteps WHERE SimID = ? AND Step = ?"
        );
        pstmt1->setInt(1, simId);
        pstmt1->setInt(2, step);

        sql::ResultSet* res1 = pstmt1->executeQuery();
        int stepId = -1;
        if (res1->next()) {
            stepId = res1->getInt("StepID");
            currentTime=res1->getDouble("CurrentTime");
        }
        delete pstmt1;
        delete res1;

        if (stepId == -1) {
            cout << "No matching step found for simId " << simId << " at step " << step << endl;
            return -1;  // 未找到对应的Step
        }

        // 2. 查询 SimulationResults 表，获取仿真结果
        sql::PreparedStatement* pstmt2 = con->prepareStatement(
            "SELECT DemandID, NodeID, NextNodeID, NextHopLinkID, AvailableKeys, RemainVolume, Status, IsRouteFailed "
            "FROM SimulationResults WHERE StepID = ?"
        );
        pstmt2->setInt(1, stepId);
        sql::ResultSet* res2 = pstmt2->executeQuery();

        // 3. 遍历结果集，将查询到的结果转换为 SimResDto 并添加到 vector 中
        while (res2->next()) {
            SimResDto dto;
            dto.demandId = res2->getInt("DemandID");
            dto.nodeId = res2->getInt("NodeID");
            dto.nextNodeId = res2->getInt("NextNodeID");
            dto.nextHopLinkId = res2->getInt("NextHopLinkID");
            dto.availableKeys = res2->getDouble("AvailableKeys");
            dto.remainVolume = res2->getDouble("RemainVolume");
            string status=res2->getString("Status");
            dto.status = status;
            string isRouteFailed=res2->getString("IsRouteFailed");
            dto.isRouteFailed = isRouteFailed;

            res.push_back(dto);
        }
        delete pstmt2;
        delete res2;

       

        cout << "Successfully retrieved simulation results for simId " << simId << " at step " << step << endl;
        return 0;  // 查询成功
    } catch (sql::SQLException& e) {
        cerr << "Error during query: " << e.what() << endl;
        return -1;  // 查询失败
    }
}

int SimDao::getSimStatus(int simId,SimStatusDto& simStatus) {
    try {
        sql::PreparedStatement* pstmt1 = con->prepareStatement(
            "SELECT Name, Status, RouteAlg, ScheduleAlg, CurrentStep, CurrentTime FROM Simulations WHERE SimID = ?"
        );
        pstmt1->setInt(1, simId);

        sql::ResultSet* res1 = pstmt1->executeQuery();
        int stepId = -1;
        if (res1->next()) {
            simStatus.name=res1->getString("Name").asStdString();
            simStatus.status=res1->getString("Status").asStdString();
            simStatus.routeAlg=res1->getString("RouteAlg").asStdString();
            simStatus.scheduleAlg=res1->getString("ScheduleAlg").asStdString();
            simStatus.currentStep=res1->getUInt("CurrentStep");
            simStatus.currentTime=res1->getDouble("CurrentTime");
        }else{
            cout << "No matching sim found for simId " << simId << endl;
            return -1;  // 未找到对应的Step
        }
        delete pstmt1;
        delete res1;

        

        return 1;
    } catch (sql::SQLException& e) {
        cerr << "Error during query: " << e.what() << endl;
        return -1;  // 查询失败
    }
}

int SimDao::setSimStatus(int simId,string status){
     try {

        // 使用PreparedStatement执行SQL语句，更新Status字段
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "UPDATE Simulations SET Status = ? WHERE SimID = ?"
        );
        pstmt->setString(1, status);  // 设置目标状态
        pstmt->setInt(2, simId);      // 设置目标SimID

        // 执行更新操作
        int rowsAffected = pstmt->executeUpdate();
        delete pstmt;

        // 关闭连接

        if (rowsAffected > 0) {
            cout << "Successfully updated status for SimID " << simId << " to " << status << endl;
            return 1;  // 更新成功
        } else {
            cout << "No simulation found with SimID " << simId << endl;
            return -1;  // 未找到对应SimID的记录
        }
    } catch (sql::SQLException& e) {
        cerr << "Error during database operation: " << e.what() << endl;
        return -1;  // 错误时返回-1
    }
}

int SimDao::setSimStepAndTime(int simId, int step, int currentTime) {
    try {

        // 使用PreparedStatement执行SQL语句，更新CurrentStep和CurrentTime字段
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "UPDATE Simulations SET CurrentStep = ?, CurrentTime = ? WHERE SimID = ?"
        );
        pstmt->setInt(1, step);        // 设置新的仿真步骤
        pstmt->setInt(2, currentTime); // 设置新的当前时间
        pstmt->setInt(3, simId);       // 设置目标SimID

        // 执行更新操作
        int rowsAffected = pstmt->executeUpdate();
        delete pstmt;

        // 关闭连接

        if (rowsAffected > 0) {
            cout << "Successfully updated step and time for SimID " << simId << " to Step " << step << " and Time " << currentTime << endl;
            return 1;  // 更新成功
        } else {
            cout << "No simulation found with SimID " << simId << endl;
            return -1;  // 未找到对应SimID的记录
        }
    } catch (sql::SQLException& e) {
        cerr << "Error during database operation: " << e.what() << endl;
        return -1;  // 错误时返回-1
    }
}

int SimDao::createSim(int simId,int groupId, string name, string routeAlg, string scheduleAlg) {
    try {

        // 插入新的仿真记录
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "INSERT INTO Simulations (SimID,GroupID, Name, Status, RouteAlg, ScheduleAlg, CurrentStep, CurrentTime) "
            "VALUES (?,?, ?, 'Init', ?, ?, 0, 0)"
        );
        pstmt->setInt(1,simId);
        pstmt->setInt(2,groupId);
        pstmt->setString(3, name);
        pstmt->setString(4, routeAlg);
        pstmt->setString(5, scheduleAlg);

        // 执行插入操作
        pstmt->executeUpdate();
        delete pstmt;

        // // 获取最后插入的SimID
        // sql::Statement* stmt = con->createStatement();
        // sql::ResultSet* res = stmt->executeQuery("SELECT LAST_INSERT_ID()");
        // int newSimId = -1;
        // if (res->next()) {
        //     newSimId = res->getInt(1);  // 获取插入的SimID
        // }
        // delete stmt;
        // delete res;

        // 关闭连接

        cout << "Successfully created simulation with SimID: " << simId << endl;
        return 1;  // 返回新插入的SimID
    } catch (sql::SQLException& e) {
        cerr << "Error during database operation: " << e.what() << endl;
        return -1;  // 错误时返回-1
    }
}