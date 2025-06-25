-- Active: 1683457944438@@127.0.0.1@3306@QKDSIM_DB
DROP DATABASE IF EXISTS QKDSIM_DB;
CREATE DATABASE IF NOT EXISTS QKDSIM_DB;

USE QKDSIM_DB;

/* Stauts: 0-初始化 1-运行中 2-暂停 3-完成 */
CREATE TABLE Simulations (
    SimID              INT PRIMARY KEY,
    GroupID            INT NOT NULL,
    Name               VARCHAR(255) NOT NULL,
    CreateTime          VARCHAR(20),
    Status             VARCHAR(31) NOT NULL,
    RouteAlg            VARCHAR(63) NOT NULL,
    ScheduleAlg          VARCHAR(63) NOT NULL,
    CurrentStep        INT UNSIGNED NOT NULL DEFAULT 0,
    CurrentTime        DOUBLE NOT NULL DEFAULT 0
);

CREATE TABLE SimulationSteps (
    StepID SERIAL PRIMARY KEY,   -- 唯一标识Step
    SimID INT REFERENCES Simulations(SimID) ON DELETE CASCADE, -- 关联仿真
    Step INT NOT NULL,           -- 当前Step编号
    CurrentTime DOUBLE PRECISION NOT NULL, -- 该Step对应的时间

    TransferredVolume DOUBLE NOT NULL DEFAULT 0,   -- 已传输数据量
    TransferredPercent DOUBLE PRECISION NOT NULL DEFAULT 0,  -- 已传输需求的百分比 (0~100)
    RemainingVolume DOUBLE NOT NULL DEFAULT 0,     -- 剩余传输数据量
    TransferRate DOUBLE PRECISION NOT NULL DEFAULT 0,    -- 传输速率
    InProgressDemandCount INT NOT NULL DEFAULT 0   -- InProgress需求数量
);

CREATE TABLE SimulationResults (
    ResID SERIAL PRIMARY KEY,  -- 结果ID
    StepID INT REFERENCES SimulationSteps(StepID) ON DELETE CASCADE, -- 关联Step
    DemandID INT NOT NULL,
    NodeID INT NOT NULL,
    NextNodeID INT NOT NULL,
    NextHopLinkID INT NOT NULL,
    AvailableKeys DOUBLE PRECISION NOT NULL,
    RemainVolume DOUBLE PRECISION NOT NULL,
    Status VARCHAR(16),
    IsRouteFailed VARCHAR(16) -- 可选值：YES/NO
);



/* CREATE TABLE SimResult(
    SimID INT UNSIGNED NOT NULL,
    Step INT UNSIGNED NOT NULL,
    Time DOUBLE NOT NULL,

    DemandID INT UNSIGNED NOT NULL,
    NodeID  INT UNSIGNED NOT NULL,
    NextNodeID INT UNSIGNED NOT NULL,
    NextHopLinkID INT UNSIGNED NOT NULL,
    AvailableKeys   DOUBLE NOT NULL,
    RemainVolume DOUBLE NOT NULL,

    Status VARCHAR NOT NULL,
    IsRouteFailed VARCHAR NOT NULL,
) */