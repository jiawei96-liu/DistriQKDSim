#include"Web/dao/SimDao.hpp"
#include"Config/ConfigReader.h"
#include"Web/Utils.hpp"
using namespace std;


SimDao::SimDao() {
    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con.reset(driver->connect(ConfigReader::getStr("db_host"),ConfigReader::getStr("user"), ConfigReader::getStr("password")));
        host=ConfigReader::getStr("db_host");
        user=ConfigReader::getStr("user");
        password= ConfigReader::getStr("password");
        database= ConfigReader::getStr("scheme");
        con->setSchema(ConfigReader::getStr("scheme")); // 选择数据库
    } catch (sql::SQLException& e) {
        std::cerr << "Database connection failed: " << e.what() << std::endl;
    }
}

SimDao::~SimDao(){

}

int SimDao::batchInsertSimulationResults(int simId,int step,double currentTime,vector<SimResultStatus> &simRes) {
    if(simRes.size()==0){
        return 1;
    }
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
        // con->setAutoCommit(false);

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
        // con->commit();

        cout << "Batch insert completed!" << endl;

        // **释放资源**
        delete stmt;
        return 1;
    } catch (sql::SQLException &e) {
        cerr << "SQL Error (batchInsertSimulationResults): " << e.what() << endl;
        return -1;
    }
}

int SimDao::batchInsertSimulationResultsAndMetric(int simId,SimMetric metric,vector<SimResultStatus> &simRes) {
    if(simRes.size()==0){
        return 1;
    }
    try {
        sql::PreparedStatement* pstmt;

        // **连接 MySQL**
        

        // **获取或创建 StepID**
        int stepID = getOrCreateStepID(simId, metric);
        if (stepID == -1) {
            cerr << "Failed to get or create StepID!" << endl;
            return -1;
        }

        // **开启事务**
        // con->setAutoCommit(false);

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
        // con->commit();

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

int SimDao::getOrCreateStepID(int simId, SimMetric metric) {
    try {
        // 查询 StepID
        auto pstmt = con->prepareStatement(
            "SELECT StepID FROM SimulationSteps WHERE SimID = ? AND Step = ?"
        );
        pstmt->setInt(1, simId);
        pstmt->setInt(2, metric.step);
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
            "INSERT INTO SimulationSteps "
            "(SimID, Step, CurrentTime, TransferredVolume, TransferredPercent, "
            "RemainingVolume, TransferRate, InProgressDemandCount) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
        );

        // 设置各字段的值
        pstmt->setInt(1, simId);
        pstmt->setInt(2, metric.step);
        pstmt->setDouble(3, metric.CurrentTime);
        pstmt->setDouble(4, metric.TransferredVolume);
        pstmt->setDouble(5, metric.TransferredPercent);
        pstmt->setDouble(6, metric.RemainingVolume);
        pstmt->setDouble(7, metric.TransferRate);
        pstmt->setInt(8, metric.InProgressDemandCount);

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
        // con->setAutoCommit(false);
        sql::Connection* newCon=driver->connect(host,user,password);
        newCon->setSchema(database);
        // 1. 删除 SimulationResults 表中的记录
        sql::PreparedStatement* pstmt1 = newCon->prepareStatement(
            "DELETE FROM SimulationResults WHERE StepID IN (SELECT StepID FROM SimulationSteps WHERE SimID = ?)"
        );
        pstmt1->setInt(1, simId);
        pstmt1->executeUpdate();
        delete pstmt1;

        // 2. 删除 SimulationSteps 表中的记录
        sql::PreparedStatement* pstmt2 = newCon->prepareStatement(
            "DELETE FROM SimulationSteps WHERE SimID = ?"
        );
        pstmt2->setInt(1, simId);
        pstmt2->executeUpdate();
        delete pstmt2;

        // 3. 删除 Simulations 表中的记录
        sql::PreparedStatement* pstmt3 = newCon->prepareStatement(
            "DELETE FROM Simulations WHERE SimID = ?"
        );
        pstmt3->setInt(1, simId);
        pstmt3->executeUpdate();
        delete pstmt3;
        delete newCon;
        // 提交事务
        // con->commit();

        // 关闭连接

        cout << "Simulation " << simId << " and its associated data have been successfully deleted!" << endl;
        return 1;// 删除成功
    } catch (sql::SQLException& e) {
        cerr << "Error during deletion: " << e.what() << endl;
        return -1;  // 删除失败
    }
}

int SimDao::clear(){
    //删除所有表项
    try {
        // 开始事务处理
        // con->setAutoCommit(false);
        sql::Connection* newCon=driver->connect(host,user,password);
        newCon->setSchema(database);
        // 1. 删除 SimulationResults 表中的所有记录
        sql::PreparedStatement* pstmt1 = newCon->prepareStatement(
            "DELETE FROM SimulationResults"
        );
        pstmt1->executeUpdate();
        delete pstmt1;
 
        // 2. 删除 SimulationSteps 表中的所有记录
        sql::PreparedStatement* pstmt2 = newCon->prepareStatement(
            "DELETE FROM SimulationSteps"
        );
        pstmt2->executeUpdate();
        delete pstmt2;
 
        // 3. 删除 Simulations 表中的所有记录
        sql::PreparedStatement* pstmt3 = newCon->prepareStatement(
            "DELETE FROM Simulations"
        );
        pstmt3->executeUpdate();
        delete pstmt3;
        delete newCon;
        // 提交事务
        // con->commit();
 
        cout << "All tables have been successfully cleared!" << endl;
        return 1; // 清除成功
    } catch (sql::SQLException& e) {
        cerr << "Error during clearing tables: " << e.what() << endl;
        // try {
        //     // 如果出现异常，尝试回滚事务
        //     if (con) {
        //         con->rollback();
        //     }
        // } catch (sql::SQLException& rollbackEx) {
        //     cerr << "Error during rollback: " << rollbackEx.what() << endl;
        // }
        return -1; // 清除失败
    }
}

int SimDao::getSimRes(int simId, int step, vector<SimResDto>& res,TIME& currentTime) {
    try {
        

        // 1. 获取 StepID
        sql::Connection* newCon=driver->connect(host,user,password);
        newCon->setSchema(database);
        sql::PreparedStatement* pstmt1 = newCon->prepareStatement(
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
        sql::PreparedStatement* pstmt2 = newCon->prepareStatement(
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
        delete newCon;
       

        cout << "Successfully retrieved simulation results for simId " << simId << " at step " << step << endl;
        return 0; // 查询成功
    } catch (sql::SQLException& e) {
        cerr << "Error during query: " << e.what() << endl;
        return -1;  // 查询失败
    }
}

int SimDao::getSimStatus(int simId,SimStatusDto& simStatus) {
    try {
        sql::Connection* newCon=driver->connect(host,user,password);
        newCon->setSchema(database);
        sql::PreparedStatement* pstmt1 = newCon->prepareStatement(
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
        delete newCon;
        

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
            return 1;// 更新成功
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
        sql::Connection* newCon=driver->connect(host,user,password);
        newCon->setSchema(database);
        sql::PreparedStatement* pstmt = newCon->prepareStatement(
            "INSERT INTO Simulations (SimID,GroupID, Name,CreateTime, Status, RouteAlg, ScheduleAlg, CurrentStep, CurrentTime) "
            "VALUES (?,?, ?,?, 'Running', ?, ?, 0, 0)"
        );
        pstmt->setInt(1,simId);
        pstmt->setInt(2,groupId);
        pstmt->setString(3, name);
        string createTime=getCurrentTimeString();
        pstmt->setString(4,createTime);
        pstmt->setString(5, routeAlg);
        pstmt->setString(6, scheduleAlg);

        // 执行插入操作
        pstmt->executeUpdate();
        delete pstmt;
        delete newCon;

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

int SimDao::getAllSims(vector<SimStatusDto>& res){
    try {
        // 建立数据库连接
        sql::Connection* newCon = driver->connect(host, user, password);
        newCon->setSchema(database);

        // 执行查询
        sql::Statement* stmt = newCon->createStatement();
        sql::ResultSet* resultSet = stmt->executeQuery("SELECT SimID, Name, CreateTime, Status, RouteAlg, ScheduleAlg, CurrentStep, CurrentTime FROM Simulations ORDER BY CreateTime DESC");

        while (resultSet->next()) {
            SimStatusDto dto;
            dto.id=resultSet->getInt("SimID");
            dto.name = resultSet->getString("Name").asStdString();
            dto.createTime = resultSet->getString("CreateTime").asStdString();
            dto.status = resultSet->getString("Status").asStdString();
            dto.routeAlg = resultSet->getString("RouteAlg").asStdString();
            dto.scheduleAlg = resultSet->getString("ScheduleAlg").asStdString();
            dto.currentStep = resultSet->getUInt("CurrentStep");
            dto.currentTime = resultSet->getDouble("CurrentTime");

            res.push_back(dto);
        }

        // 清理资源
        delete resultSet;
        delete stmt;
        delete newCon;

        return 1;
    } catch (sql::SQLException& e) {
        cerr << "Error fetching simulations: " << e.what() << endl;
        return -1;
    }
}

int SimDao::exportSimResToStream(int simId, std::stringstream& ss) {
    try {
        sql::Connection* newCon = driver->connect(host, user, password);
        newCon->setSchema(database);

        sql::PreparedStatement* pstmt1 = newCon->prepareStatement(
            "SELECT StepID, Step, CurrentTime FROM SimulationSteps WHERE SimID = ? ORDER BY Step ASC"
        );
        pstmt1->setInt(1, simId);
        sql::ResultSet* res1 = pstmt1->executeQuery();

        while (res1->next()) {
            int stepId = res1->getInt("StepID");
            int step = res1->getInt("Step");
            double currentTime = res1->getDouble("CurrentTime");

            sql::PreparedStatement* pstmt2 = newCon->prepareStatement(
                "SELECT DemandID, NodeID, NextNodeID, NextHopLinkID, AvailableKeys, RemainVolume, Status, IsRouteFailed "
                "FROM SimulationResults WHERE StepID = ?"
            );
            pstmt2->setInt(1, stepId);
            sql::ResultSet* res2 = pstmt2->executeQuery();

            while (res2->next()) {
                ss << step << "," << currentTime << ","
                   << res2->getInt("DemandID") << ","
                   << res2->getInt("NodeID") << ","
                   << res2->getInt("NextNodeID") << ","
                   << res2->getInt("NextHopLinkID") << ","
                   << res2->getDouble("AvailableKeys") << ","
                   << res2->getDouble("RemainVolume") << ","
                   << res2->getString("Status") << ","
                   << res2->getString("IsRouteFailed") << "\n";
            }

            delete res2;
            delete pstmt2;
        }

        delete res1;
        delete pstmt1;
        delete newCon;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Export error: " << e.what() << std::endl;
        return -1;
    }
}

int SimDao::getSimMetric(int simId,int step,SimMetricDto& ret){
    try {
        std::unique_ptr<sql::Connection> con(driver->connect(host, user, password));
        con->setSchema(database);

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT StepID, SimID, Step, CurrentTime, "
                "TransferredVolume, TransferredPercent, RemainingVolume, "
                "TransferRate, InProgressDemandCount "
                "FROM SimulationSteps WHERE SimID = ? AND Step = ?"
            )
        );

        pstmt->setInt(1, simId);
        pstmt->setInt(2, step);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {
            ret.stepId = res->getInt("StepID");
            ret.simId = res->getInt("SimID");
            ret.step = res->getInt("Step");
            ret.currentTime = res->getDouble("CurrentTime");
            ret.transferredVolume = res->getDouble("TransferredVolume");
            ret.transferredPercent = res->getDouble("TransferredPercent");
            ret.remainingVolume = res->getDouble("RemainingVolume");
            ret.transferRate = res->getDouble("TransferRate");
            ret.inProgressDemandCount = res->getDouble("InProgressDemandCount");
            return 1; // 成功
        } else {
            return 0; // 未找到对应记录
        }

    } catch (sql::SQLException &e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return -1; // 数据库异常
    }
    
}

int SimDao::setSimRunningStatusToEnd(){
    try {
        
        // 使用PreparedStatement执行SQL语句，更新Status字段
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "UPDATE Simulations SET Status = ? WHERE Status = ?"
        );
        pstmt->setString(1, "End");  // 设置目标状态
        pstmt->setString(2, "Running");      // 设置目标SimID

        // 执行更新操作
        int rowsAffected = pstmt->executeUpdate();
        delete pstmt;

        // 关闭连接

        if (rowsAffected > 0) {
            cout << "Successfully updated status " << endl;
            return 1;  // 更新成功
        } else {
            cout << "No running simulation found " << endl;
            return 1;  // 
        }
    } catch (sql::SQLException& e) {
        cerr << "Error during database operation: " << e.what() << endl;
        return -1;  // 错误时返回-1
    }
}