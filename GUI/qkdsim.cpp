﻿
#include "qkdsim.h"
#include "ui_qkdsim.h"


QKDSim::QKDSim(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QKDSim)
{
    ui->setupUi(this);
    ui->tableWidget_net->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  // 表格列宽自动伸缩
    ui->tableWidget_net->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_dem->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_dem->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_path->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_path->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_out->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_out->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_link->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_link->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_node->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_node->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 读取csv文件
    loadCSV("../Input/network.csv", Network);
    loadCSV("../Input/demand.csv", Demand);
    timer = new QTimer(this);

    Connections();

//    net = nullptr;
    net = new CNetwork();

    scene = new QGraphicsScene(this);
    ui->graph_node->setScene(scene);
}

QKDSim::~QKDSim()
{
    delete net;
    delete ui;
}

void QKDSim::Connections()
{
    // 关于QT
    connect(ui->action_qt, &QAction::triggered, [this] {QMessageBox::aboutQt(this);});

//    // 帮助
//    connect(ui->action_help, &QAction::triggered, help_dialog, &QWidget::show);

    // 关闭程序
    connect(ui->action_exit, &QAction::triggered, this, &QWidget::close);

    // 保存打开文件（接收内容显示框）
    connect(ui->action_open_net, &QAction::triggered, this, &QKDSim::open_net);
    connect(ui->action_open_dem, &QAction::triggered, this, &QKDSim::open_dem);
    connect(ui->action_save_net, &QAction::triggered, this, &QKDSim::save_net);
    connect(ui->action_save_dem, &QAction::triggered, this, &QKDSim::save_dem);

    // 定时器
    connect(timer, &QTimer::timeout, this, &QKDSim::next_step);
}

void QKDSim::loadCSV(const QString &fileName, Kind kind)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream in(&file);

    QTableWidget *tableWidget;

    QStringList headers;

    switch (kind)
    {
    case Network:
        tableWidget = ui->tableWidget_net;
        /********* 读取node个数 ************/
        int nodeNum;
        in >> nodeNum;
        ui->edit_node_num->setText(QString::number(nodeNum));   // 将显示的nodeNum也更改
        in.readLine();  // 略过第一行的换行符

        headers = {"linkId", "sourceId", "sinkId", "keyRate", "proDelay", "bandWidth", "weight", "faultTime"};
        break;

    case Demand:
        tableWidget = ui->tableWidget_dem;

        headers = {"demandId", "sourceId", "sinkId", "demandVolume", "arriveTime"};
        break;

    default:
        break;
    }

    // 清空表格
    tableWidget->clear();
    tableWidget->setRowCount(0);

    tableWidget->setColumnCount(headers.size());
    tableWidget->setHorizontalHeaderLabels(headers);

    int row = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList fields = line.split(",");

        tableWidget->insertRow(row);
        for (int column = 0; column < fields.size(); ++column)
        {
            tableWidget->setItem(row, column, new QTableWidgetItem(fields[column]));
        }
        row++;
    }

    file.close();
}

void QKDSim::open_net()
{
    // 加载文件至接收内容显示框
    QString filename = QFileDialog::getOpenFileName(this, tr("打开网络拓扑文件"));
    loadCSV(filename, Network);
    ui->tabWidget->setCurrentIndex(0);
}

void QKDSim::open_dem()
{
    // 加载文件至接收内容显示框
    QString filename = QFileDialog::getOpenFileName(this, tr("打开需求文件"));
    loadCSV(filename, Demand);
    ui->tabWidget->setCurrentIndex(1);
}

void QKDSim::save_net()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("保存网络拓扑"));
    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
    {
        ui->statusbar->showMessage("Save file failed", 5000);
        return;
    }

    QTextStream out(&file);
    out << ui->edit_node_num->text().toInt() << "\n"; // 第一行存nodenum
    for (int row = 0; row < ui->tableWidget_net->rowCount(); ++row)
    {
        QStringList rowData;
        for (int column = 0; column < ui->tableWidget_net->columnCount(); ++column)
        {
            QTableWidgetItem *item = ui->tableWidget_net->item(row, column);
            rowData << (item ? item->text() : "");
        }
        out << rowData.join(",") << "\n";
    }
    file.close();
    ui->statusbar->showMessage("Successfully save network", 5000);
}

void QKDSim::save_dem()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("保存网络需求"));
    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
    {
        ui->statusbar->showMessage("Save file failed", 5000);
        return;
    }

    QTextStream out(&file);
    for (int row = 0; row < ui->tableWidget_dem->rowCount(); ++row)
    {
        QStringList rowData;
        for (int column = 0; column < ui->tableWidget_dem->columnCount(); ++column)
        {
            QTableWidgetItem *item = ui->tableWidget_dem->item(row, column);
            rowData << (item ? item->text() : "");
        }
        out << rowData.join(",") << "\n";
    }
    file.close();
    ui->statusbar->showMessage("Successfully save demand", 5000);
}

void QKDSim::readNetTable()
{
    // 读取节点信息
    QString nodeNumString = ui->edit_node_num->text();
    if (nodeNumString.isEmpty())
    {
        ui->statusbar->showMessage("Error: Missing data in node number", 5000);
        return;
    }
    int nodeNum = nodeNumString.toInt();
    net->SetNodeNum(nodeNum);
    net->InitNodes(nodeNum);

    // 逐行读取链路信息
    for (int row = 0; row < ui->tableWidget_net->rowCount(); ++row)
    {
        LINKID linkId;
        NODEID sourceId, sinkId;
        RATE keyRate, bandWidth;
        TIME proDelay, faultTime;
        WEIGHT weight;

        // 从表格的每一列读取数据
        if (ui->tableWidget_net->item(row, 0) && ui->tableWidget_net->item(row, 1) && ui->tableWidget_net->item(row, 2) && ui->tableWidget_net->item(row, 3)
                && ui->tableWidget_net->item(row, 4) && ui->tableWidget_net->item(row, 5) && ui->tableWidget_net->item(row, 6))
        {
            // 转换为相应的数据类型
            linkId = ui->tableWidget_net->item(row, 0)->text().toUInt();
            sourceId = ui->tableWidget_net->item(row, 1)->text().toUInt();
            sinkId = ui->tableWidget_net->item(row, 2)->text().toUInt();
            keyRate = ui->tableWidget_net->item(row, 3)->text().toDouble();  // 假设 keyRate 是一个双精度浮点数
            proDelay = ui->tableWidget_net->item(row, 4)->text().toDouble(); // 假设 proDelay 是一个双精度浮点数
            bandWidth = ui->tableWidget_net->item(row, 5)->text().toDouble();
            weight = ui->tableWidget_net->item(row, 6)->text().toDouble();
            faultTime = ui->tableWidget_net->item(row, 7)->text() == "" ? -1 : ui->tableWidget_net->item(row, 7)->text().toDouble();    // 没有故障，则为-1

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

            net->m_vAllLinks.push_back(newLink);
            net->m_mNodePairToLink[make_pair(sourceId, sinkId)] = linkId;
            net->m_mNodePairToLink[make_pair(sinkId, sourceId)] = linkId;
            net->InitKeyManagerOverLink(linkId);


            net->m_vAllNodes[sourceId].m_lAdjNodes.push_back(sinkId);
            net->m_vAllNodes[sinkId].m_lAdjNodes.push_back(sourceId);
            net->InitKeyManagerOverLink(linkId);

            /**********如何赋值**************/
            int id_faultTime = 1000000 + linkId;    // m_mDemandArriveTime插入的value为1000000 + linkId
            if (faultTime >= 0)
                net->m_mDemandArriveTime.insert(make_pair(faultTime, id_faultTime));   // 增加m_mDemandArriveTime视为增加故障点
        }
        else
        {
            ui->statusbar->showMessage("Error: Missing data in network table", 5000);
        }
    }
    net->SetLinkNum(ui->tableWidget_net->rowCount() - 1); //第一行是link数量，需要rowCount()-1
    ui->statusbar->showMessage("Network data processed successfully", 5000);
}

void QKDSim::readDemTable()
{
    // 逐行读取需求信息
    for (int row = 0; row < ui->tableWidget_dem->rowCount(); ++row)
    {
        DEMANDID demandId;
        NODEID sourceId, sinkId;
        VOLUME demandVolume;
        TIME arriveTime;

        // 从表格的每一列读取数据
        QTableWidgetItem *demandIdItem = ui->tableWidget_dem->item(row, 0);
        QTableWidgetItem *sourceIdItem = ui->tableWidget_dem->item(row, 1);
        QTableWidgetItem *sinkIdItem = ui->tableWidget_dem->item(row, 2);
        QTableWidgetItem *demandVolumeItem = ui->tableWidget_dem->item(row, 3);
        QTableWidgetItem *arriveTimeItem = ui->tableWidget_dem->item(row, 4);

        if (demandIdItem && sourceIdItem && sinkIdItem && demandVolumeItem && arriveTimeItem)
        {
            // 转换为相应的数据类型
            demandId = demandIdItem->text().toUInt();
            sourceId = sourceIdItem->text().toUInt();
            sinkId = sinkIdItem->text().toUInt();
            demandVolume = demandVolumeItem->text().toDouble();  // 假设 demandVolume 是一个双精度浮点数
            arriveTime = arriveTimeItem->text().toDouble();  // 假设 arriveTime 是一个双精度浮点数

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
        else
        {
            ui->statusbar->showMessage("Error: Missing data in demand table", 5000);
        }
    }


    net->SetDemandNum(ui->tableWidget_dem->rowCount() - 1); //第一行是demand数量，需要rowCount()-1
    ui->statusbar->showMessage("Demand data processed successfully", 5000);
}

void QKDSim::showOutput()
{
    ui->edit_time->setText(QString::number(net->CurrentTime(), 'f', 2));

    ui->tableWidget_out->clear();
    ui->tableWidget_out->setRowCount(0);    // 清空表格
    QStringList headers = {"demandId", "nodeId", "nextNode", "minLink", "avaiableKeys", "relayVolume", "isDelivered", "isFailed"};     // 尚未确定
    ui->tableWidget_out->setColumnCount(headers.size());
    ui->tableWidget_out->setHorizontalHeaderLabels(headers);

    // 遍历每个节点上正在传输的数据量
    for (NODEID nodeId = 0; nodeId < net->GetNodeNum(); nodeId++)
    {
        for (auto demandIter = net->m_vAllNodes[nodeId].m_mRelayVolume.begin(); demandIter != net->m_vAllNodes[nodeId].m_mRelayVolume.end();)
        {
            if (net->m_vAllDemands[demandIter->first].GetRoutedFailed())
            {
                demandIter = net->m_vAllNodes[nodeId].m_mRelayVolume.erase(demandIter);
                continue;
            }
            DEMANDID demandId = demandIter->first;
            VOLUME relayVolume = demandIter->second;
            bool isDelivered = net->m_vAllDemands[demandId].GetAllDelivered();
            // 对于每个需求，从其路径中找到下一个要中继到的节点 nextNode
            NODEID nextNode = net->m_vAllDemands[demandId].m_Path.m_mNextNode[nodeId];
            // 找到当前节点和下一个节点之间的链路 minLink
            LINKID minLink = net->m_mNodePairToLink[make_pair(nodeId, nextNode)];
            VOLUME avaiableKeys = net->m_vAllLinks[minLink].GetAvaialbeKeys();
            bool isRouteFailed = net->m_vAllDemands[demandId].GetRoutedFailed();

            int newRow = ui->tableWidget_out->rowCount();
            ui->tableWidget_out->insertRow(newRow);    // 末尾增加一行

            ui->tableWidget_out->setItem(newRow, 0, new QTableWidgetItem(QString::number(demandId)));
            ui->tableWidget_out->setItem(newRow, 1, new QTableWidgetItem(QString::number(nodeId)));
            ui->tableWidget_out->setItem(newRow, 2, new QTableWidgetItem(QString::number(nextNode)));
            ui->tableWidget_out->setItem(newRow, 3, new QTableWidgetItem(QString::number(minLink)));
            ui->tableWidget_out->setItem(newRow, 4, new QTableWidgetItem(QString::number(avaiableKeys, 'f', 2)));
            ui->tableWidget_out->setItem(newRow, 5, new QTableWidgetItem(QString::number(relayVolume, 'f', 2)));
            ui->tableWidget_out->setItem(newRow, 6, new QTableWidgetItem(isDelivered ? "True" : "False"));
            ui->tableWidget_out->setItem(newRow, 7, new QTableWidgetItem(isRouteFailed ? "True" : "False"));

            demandIter++;
        }
    }
    // 显示已传输完毕的数据
    for (auto demandIter = net->m_vAllDemands.begin(); demandIter != net->m_vAllDemands.end(); demandIter++)
    {
        if (demandIter->GetAllDelivered())
        {
            NODEID nodeId = demandIter->GetSinkId();
            DEMANDID demandId = demandIter->GetDemandId();
            VOLUME relayVolume = 0;
            bool isDelivered = demandIter->GetAllDelivered();
//            NODEID nextNode = demandIter->GetSinkId();
//            LINKID minLink = net->m_mNodePairToLink[make_pair(nodeId, nextNode)];
//            VOLUME avaiableKeys = net->m_vAllLinks[minLink].GetAvaialbeKeys();
            bool isRouteFailed = net->m_vAllDemands[demandId].GetRoutedFailed();

            int newRow = ui->tableWidget_out->rowCount();
            ui->tableWidget_out->insertRow(newRow);    // 末尾增加一行

            ui->tableWidget_out->setItem(newRow, 0, new QTableWidgetItem(QString::number(demandId)));
            ui->tableWidget_out->setItem(newRow, 1, new QTableWidgetItem(QString::number(nodeId)));
            ui->tableWidget_out->setItem(newRow, 2, new QTableWidgetItem());
            ui->tableWidget_out->setItem(newRow, 3, new QTableWidgetItem());
            ui->tableWidget_out->setItem(newRow, 4, new QTableWidgetItem());
            ui->tableWidget_out->setItem(newRow, 5, new QTableWidgetItem(QString::number(relayVolume, 'f', 2)));
            ui->tableWidget_out->setItem(newRow, 6, new QTableWidgetItem(isDelivered ? "True" : "False"));
            ui->tableWidget_out->setItem(newRow, 7, new QTableWidgetItem(isRouteFailed ? "True" : "False"));
        }
    }

    ui->tableWidget_path->clear();
    ui->tableWidget_path->setRowCount(0);    // 清空表格
    QStringList headers_path = {"demandId", "Node1", "Node2"};     // 尚未确定
    ui->tableWidget_path->setColumnCount(headers_path.size());
    ui->tableWidget_path->setHorizontalHeaderLabels(headers_path);

    // 显示每个需求的路由的最短路径
    for (auto demandIter = net->m_vAllDemands.begin(); demandIter != net->m_vAllDemands.end(); demandIter++)
    {
        if (demandIter->GetRoutedFailed())
        {
            DEMANDID demandId = demandIter->GetDemandId();
            int newRow = ui->tableWidget_path->rowCount();
            ui->tableWidget_path->insertRow(newRow);    // 末尾增加一行
            ui->tableWidget_path->setItem(newRow, 0, new QTableWidgetItem(QString::number(demandId)));
            // 都显示-1
            ui->tableWidget_path->setItem(newRow, 1, new QTableWidgetItem("-1"));
            ui->tableWidget_path->setItem(newRow, 2, new QTableWidgetItem("-1"));
        }
        else
        {
            DEMANDID demandId = demandIter->GetDemandId();
            list<NODEID> node_path = net->m_vAllDemands[demandId].m_Path.m_lTraversedNodes;

            // 如果当前行需要的列数大于表格当前列数，则扩展表格的列数
            int currentColCount = static_cast<int>(node_path.size() + 1); // 因为第一列是demandId
            if (currentColCount > ui->tableWidget_path->columnCount())
            {
                int oldColCount = ui->tableWidget_path->columnCount();
                ui->tableWidget_path->setColumnCount(currentColCount);
                // 设置新的列头
                for (int col = oldColCount; col < currentColCount; ++col)
                {
                    ui->tableWidget_path->setHorizontalHeaderItem(col, new QTableWidgetItem(QString("Node%1").arg(col)));
                }
            }

            int newRow = ui->tableWidget_path->rowCount();
            ui->tableWidget_path->insertRow(newRow);    // 末尾增加一行
            // 填充当前行的数据
            ui->tableWidget_path->setItem(newRow, 0, new QTableWidgetItem(QString::number(demandId)));
            int col = 1;  // 索引变量
            for (auto it = node_path.begin(); it != node_path.end(); ++it, ++col)
            {
                ui->tableWidget_path->setItem(newRow, col, new QTableWidgetItem(QString::number(*it)));
            }
        }
    }
}

void QKDSim::on_bt_start_clicked()
{
    // 清空上一次的数据
//    if (net != nullptr)
//        delete net;
//    net = new CNetwork();
    net->Clear();

    readNetTable();
    readDemTable();

//    net->MainProcess();
    net->InitRelayPath();

    // 输出表格
    showOutput();

    if (ui->bt_route1->isChecked())
        net->setShortestPath();
    else
        net->setKeyRateShortestPath();
    if (ui->bt_schedule1->isChecked())
        net->setMinimumRemainingTimeFirst();
    else
        net->setAverageKeyScheduling();
}

void QKDSim::next_step()
{
    if (!net->AllDemandsDelivered())
    {
        TIME executeTime = net->OneTimeRelay();
        net->MoveSimTime(executeTime);

        showOutput();
        ui->statusbar->showMessage(QString("Now is %1 step").arg(net->CurrentStep()), 5000);
    }
    else
    {
        ui->statusbar->showMessage(QString("All demand has benn delivered, the end step is %1").arg(net->CurrentStep()), 5000);
    }
}

void QKDSim::on_bt_begin_clicked()
{
    if (timer->isActive())
    {
        ui->bt_begin->setText("开始");
        timer->stop();
    }
    else
    {
        ui->bt_begin->setText("暂停");
        timer->start(1000);
    }
}

void QKDSim::on_bt_next_clicked()
{
    next_step();
}

void QKDSim::on_bt_next10_clicked()
{
    for (int i = 0; i < 10; i++)
        next_step();
}

void QKDSim::on_bt_next100_clicked()
{
    for (int i = 0; i < 100; i++)
        next_step();
}

// 自动加减行
void tableWidget_cellChanged(int row, int column, QTableWidget* tableWidget)

{
    // 最后一行添加数据，则再加一个空行
    int rowCount = tableWidget->rowCount();
    if (row == rowCount - 1)
    {
        tableWidget->insertRow(rowCount);
    }

    // 某一行删除数据，查看此行是否为空行，并删除
    QTableWidgetItem* item = tableWidget->item(row, column);
    if (item != nullptr && item->text().isEmpty())
    {
        // 如果单元格为空，检查整行是否也为空
        bool isRowEmpty = true;
        for (int j = 0; j < tableWidget->columnCount(); ++j)
        {
            QTableWidgetItem* cell = tableWidget->item(row, j);
            if (cell != nullptr && !cell->text().isEmpty())
            {
                isRowEmpty = false;
                break;
            }
        }
        if (isRowEmpty)
        {
            tableWidget->removeRow(row);  // 如果整行为空，则删除该行
        }
    }
}

void QKDSim::showNodeGraph()
{
    scene->clear();
    NODEID nodeId = ui->edit_show_node->text().toInt();
    int WIDTH = ui->graph_node->size().width() - 10;
    int HEIGHT = ui->graph_node->size().height() - 10;
    qreal RADIUS = min(WIDTH, HEIGHT) / 3;
    int NODE_SIZE = min(WIDTH, HEIGHT) / 10;

    scene->setSceneRect(0, 0, WIDTH, HEIGHT);  // 场景大小
    vector<NODEID> nodeShow;
    vector<LINKID> linkShow;

    // 中心节点
    nodeShow.emplace_back(nodeId);
    scene->addEllipse(WIDTH / 2 - NODE_SIZE / 2, HEIGHT / 2 - NODE_SIZE / 2, NODE_SIZE, NODE_SIZE, QPen(), QBrush(Qt::cyan));
    QGraphicsTextItem* centerNodeText = scene->addText(QString::number(nodeId), QFont("Arial", 24));
    centerNodeText->setPos(WIDTH / 2 - centerNodeText->boundingRect().width() / 2, HEIGHT / 2 - centerNodeText->boundingRect().height() / 2);

    map<NODEID, LINKID> perNode1;   // 第一层节点
    for (auto linkIter = net->m_vAllLinks.begin(); linkIter != net->m_vAllLinks.end(); linkIter++)
    {
        if (linkIter->GetSourceId() == nodeId)
            perNode1.insert(make_pair(linkIter->GetSinkId(), linkIter->GetLinkId()));
        if (linkIter->GetSinkId() == nodeId)
            perNode1.insert(make_pair(linkIter->GetSourceId(), linkIter->GetLinkId()));
    }
    map<NODEID, pair<int, int>> loc;
    int numPeripheralNodes = static_cast<int>(perNode1.size());
    auto iter = perNode1.begin();
    for (int i = 0; i < numPeripheralNodes; i++, iter++)
    {
//        nodeShow.emplace_back(iter->first);
        linkShow.emplace_back(iter->second);
        qreal angle = 2 * M_PI * i / numPeripheralNodes;  // 分割圆周
        qreal x = WIDTH / 2 + RADIUS * cos(angle);
        qreal y = HEIGHT / 2 + RADIUS * sin(angle);
        scene->addEllipse(x - NODE_SIZE / 2, y - NODE_SIZE / 2, NODE_SIZE, NODE_SIZE, QPen(), QBrush(Qt::lightGray));
        QGraphicsTextItem* peripheralNodeText = scene->addText(QString::number(iter->first), QFont("Arial", 24)); // 给每个节点一个编号
        peripheralNodeText->setPos(x - peripheralNodeText->boundingRect().width() / 2, y - peripheralNodeText->boundingRect().height() / 2);
        // 链路
        scene->addLine(WIDTH / 2, HEIGHT / 2, x, y, QPen(Qt::gray));
        LINKID link = iter->second;
        QGraphicsTextItem* lineText = scene->addText(QString::number(link), QFont("Arial", 20)); // 线上也显示编号
        lineText->setPos((WIDTH / 2 + x) / 2, (HEIGHT / 2 + y) / 2);

        // 位置
        loc[iter->first] = make_pair(x, y);
    }
    // 第一层节点之间的链路
    for (auto i = perNode1.begin(); i != perNode1.end(); i++)
    {
        auto next = i;
        for(auto j = ++next; j != perNode1.end(); j++)
        {
            if(net->m_mNodePairToLink.count(make_pair(i->first, j->first)))
            {
                qreal x1 = loc[i->first].first;
                qreal y1 = loc[i->first].second;
                qreal x2 = loc[j->first].first;
                qreal y2 = loc[j->first].second;
                scene->addLine(x1, y1, x2, y2, QPen(Qt::gray));
                LINKID link = net->m_mNodePairToLink[make_pair(i->first, j->first)];
                QGraphicsTextItem* lineText = scene->addText(QString::number(link), QFont("Arial", 20)); // 线上也显示编号
                lineText->setPos((x1 + x2) / 2, (y1 + y2) / 2);
            }
        }
    }

    ui->graph_node->show();

    // 显示node
    ui->tableWidget_node->clear();
    ui->tableWidget_node->setRowCount(0);    // 清空表格
    QStringList headers = {"demandId", "nodeId", "minLink", "relayVolume"};     // 尚未确定
    ui->tableWidget_node->setColumnCount(headers.size());
    ui->tableWidget_node->setHorizontalHeaderLabels(headers);
    for (NODEID nodeId : nodeShow)
    {
        for (auto demandIter = net->m_vAllNodes[nodeId].m_mRelayVolume.begin(); demandIter != net->m_vAllNodes[nodeId].m_mRelayVolume.end(); demandIter++)
        {
//            if (net->m_vAllDemands[demandIter->first].GetRoutedFailed()||net->m_vAllDemands[demandIter->first].GetAllDelivered())
//            {
//                continue;
//            }
            DEMANDID demandId = demandIter->first;
            VOLUME relayVolume = demandIter->second;
            // 对于每个需求，从其路径中找到下一个要中继到的节点 nextNode
            NODEID nextNode = net->m_vAllDemands[demandId].m_Path.m_mNextNode[nodeId];
            // 找到当前节点和下一个节点之间的链路 minLink
            LINKID minLink = net->m_mNodePairToLink[make_pair(nodeId, nextNode)];

            int newRow = ui->tableWidget_node->rowCount();
            ui->tableWidget_node->insertRow(newRow);    // 末尾增加一行

            ui->tableWidget_node->setItem(newRow, 0, new QTableWidgetItem(QString::number(demandId)));
            ui->tableWidget_node->setItem(newRow, 1, new QTableWidgetItem(QString::number(nodeId)));
            ui->tableWidget_node->setItem(newRow, 2, new QTableWidgetItem(QString::number(minLink)));
            ui->tableWidget_node->setItem(newRow, 3, new QTableWidgetItem(QString::number(relayVolume, 'f', 2)));
        }
    }
    // 显示link
    ui->tableWidget_link->clear();
    ui->tableWidget_link->setRowCount(0);    // 清空表格
    QStringList headers_2 = {"linkId", "nodeId", "nextNode", "avaiableKeys"};     // 尚未确定
    ui->tableWidget_link->setColumnCount(headers_2.size());
    ui->tableWidget_link->setHorizontalHeaderLabels(headers_2);
    for (LINKID linkId : linkShow)
    {
        NODEID nodeId = net->m_vAllLinks[linkId].GetSourceId();
        NODEID nextNode = net->m_vAllLinks[linkId].GetSinkId();
        VOLUME avaiableKeys = net->m_vAllLinks[linkId].GetAvaialbeKeys();

        int newRow = ui->tableWidget_link->rowCount();
        ui->tableWidget_link->insertRow(newRow);    // 末尾增加一行

        ui->tableWidget_link->setItem(newRow, 0, new QTableWidgetItem(QString::number(linkId)));
        ui->tableWidget_link->setItem(newRow, 1, new QTableWidgetItem(QString::number(nodeId)));
        ui->tableWidget_link->setItem(newRow, 2, new QTableWidgetItem(QString::number(nextNode)));
        ui->tableWidget_link->setItem(newRow, 3, new QTableWidgetItem(QString::number(avaiableKeys, 'f', 2)));
    }
}

void QKDSim::on_tableWidget_net_cellChanged(int row, int column)
{
    tableWidget_cellChanged(row, column, ui->tableWidget_net);
}

void QKDSim::on_tableWidget_dem_cellChanged(int row, int column)
{
    tableWidget_cellChanged(row, column, ui->tableWidget_dem);
}

void QKDSim::on_bt_show_node_clicked()
{
    showNodeGraph();
}
