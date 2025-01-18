
// #include "qkdsim.h"
// #include "ui_qkdsim.h"
// #include <unordered_set>
// #include <QtConcurrent>

// QKDSim::QKDSim(QWidget *parent)
//     : QMainWindow(parent)
//     , ui(new Ui::QKDSim)
// {
//     ui->setupUi(this);
//     ui->tableWidget_net->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  // 表格列宽自动伸缩
//     ui->tableWidget_net->verticalHeader()->setDefaultSectionSize(20);
//     ui->tableWidget_dem->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//     ui->tableWidget_dem->verticalHeader()->setDefaultSectionSize(20);
//     ui->tableWidget_path->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//     ui->tableWidget_path->verticalHeader()->setDefaultSectionSize(20);
//     ui->tableWidget_out->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//     ui->tableWidget_out->verticalHeader()->setDefaultSectionSize(20);
//     ui->tableWidget_link->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//     ui->tableWidget_link->verticalHeader()->setDefaultSectionSize(20);
//     ui->tableWidget_node->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//     ui->tableWidget_node->verticalHeader()->setDefaultSectionSize(20);

//     // 读取csv文件
//     loadCSV("../../Input/network.csv", Network);
//     loadCSV("../../Input/demand.csv", Demand);
//     timer = new QTimer(this);

//     Connections();
//     // 设置CNetwork
//     net = new CNetwork();
//     // 设置拓扑图
//     scene = new QGraphicsScene(this);
//     ui->graph_node->setScene(scene);
//     // 进度条界面
//     loadingDialog = new LoadingDialog(this);
// //    connect(this, &QKDSim::computationDone, this, [loadingDialog]()
// //    {
// //        loadingDialog->accept();  // 关闭对话框
// //    });

// //    connect(this, &QKDSim::startComputation, this, [loadingDialog]()
// //    {
// //        loadingDialog->exec();  // 显示模态对话框
// //    });
// //    progressBar = new ProgressBar(this);
// }

// QKDSim::~QKDSim()
// {
//     delete net;
//     delete ui;
// }

// void QKDSim::Connections()
// {
//     // 关于QT
//     connect(ui->action_qt, &QAction::triggered, [this] {QMessageBox::aboutQt(this);});

//     // 关闭程序
//     connect(ui->action_exit, &QAction::triggered, this, &QWidget::close);

//     // 保存打开文件（接收内容显示框）
//     connect(ui->action_open_net, &QAction::triggered, this, &QKDSim::open_net);
//     connect(ui->action_open_dem, &QAction::triggered, this, &QKDSim::open_dem);
//     connect(ui->action_save_net, &QAction::triggered, this, &QKDSim::save_net);
//     connect(ui->action_save_dem, &QAction::triggered, this, &QKDSim::save_dem);

//     // 定时器
// //    connect(timer, &QTimer::timeout, this, &QKDSim::next_step);
//     connect(timer, &QTimer::timeout, this, [this]()
//     {
//         QFuture<void> future = QtConcurrent::run([this]()
//         {
//             QMetaObject::invokeMethod(timer, "stop");  // 定时器阻塞期间不运行
//             this->next_step();
//             showOutput();
//             QMetaObject::invokeMethod(timer, "start", Q_ARG(int, 1000));
//         });
//         QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
//         connect(watcher, &QFutureWatcher<void>::started, loadingDialog, &QDialog::exec);
//         connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]()
//         {
//             loadingDialog->accept(); // 关闭加载对话框
//             watcher->deleteLater();  // 清理 watcher
//         });
//         watcher->setFuture(future);
//     });

//     // 进度条界面
// //    connect(this, &QKDSim::progressChanged, progressBar, &ProgressBar::updateProgress);
// }

// void QKDSim::loadCSV(const QString &fileName, Kind kind)
// {
//     QFile file(fileName);
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//     {
//         return;
//     }

//     QTextStream in(&file);

//     QTableWidget *tableWidget;

//     QStringList headers;

//     switch (kind)
//     {
//     case Network:
//         tableWidget = ui->tableWidget_net;
//         /********* 读取node个数 ************/
//         int nodeNum;
//         in >> nodeNum;
//         ui->edit_node_num->setText(QString::number(nodeNum));   // 将显示的nodeNum也更改
//         in.readLine();  // 略过第一行的换行符

//         headers = {"链路ID", "源节点", "目的节点", "密钥生成速率", "时延", "带宽", "链路权重", "故障时间"};
//         break;

//     case Demand:
//         tableWidget = ui->tableWidget_dem;

//         headers = {"需求ID", "源节点", "目的节点", "数据量", "到达时间"};
//         break;

//     default:
//         break;
//     }

//     // 清空表格
//     tableWidget->clear();
//     tableWidget->setRowCount(0);

//     tableWidget->setColumnCount(headers.size());
//     tableWidget->setHorizontalHeaderLabels(headers);

//     int row = 0;
//     while (!in.atEnd())
//     {
//         QString line = in.readLine();
//         QStringList fields = line.split(",");

//         tableWidget->insertRow(row);
//         for (int column = 0; column < fields.size(); ++column)
//         {
//             tableWidget->setItem(row, column, new QTableWidgetItem(fields[column]));
//         }
//         row++;
//     }

//     file.close();
// }

// void QKDSim::open_net()
// {
//     // 加载文件至接收内容显示框
//     QString filename = QFileDialog::getOpenFileName(this, tr("打开网络拓扑文件"));
//     loadCSV(filename, Network);
//     ui->tabWidget->setCurrentIndex(0);
// }

// void QKDSim::open_dem()
// {
//     // 加载文件至接收内容显示框
//     QString filename = QFileDialog::getOpenFileName(this, tr("打开需求文件"));
//     loadCSV(filename, Demand);
//     ui->tabWidget->setCurrentIndex(1);
// }

// void QKDSim::save_net()
// {
//     QString filename = QFileDialog::getSaveFileName(this, tr("保存网络拓扑"));
//     QFile file(filename);
//     if (!file.open(QFile::WriteOnly))
//     {
//         ui->statusbar->showMessage("Save file failed", 5000);
//         return;
//     }

//     QTextStream out(&file);
//     out << ui->edit_node_num->text().toInt() << "\n"; // 第一行存nodenum
//     for (int row = 0; row < ui->tableWidget_net->rowCount(); ++row)
//     {
//         QStringList rowData;
//         for (int column = 0; column < ui->tableWidget_net->columnCount(); ++column)
//         {
//             QTableWidgetItem *item = ui->tableWidget_net->item(row, column);
//             rowData << (item ? item->text() : "");
//         }
//         out << rowData.join(",") << "\n";
//     }
//     file.close();
//     ui->statusbar->showMessage("Successfully save network", 5000);
// }

// void QKDSim::save_dem()
// {
//     QString filename = QFileDialog::getSaveFileName(this, tr("保存网络需求"));
//     QFile file(filename);
//     if (!file.open(QFile::WriteOnly))
//     {
//         ui->statusbar->showMessage("Save file failed", 5000);
//         return;
//     }

//     QTextStream out(&file);
//     for (int row = 0; row < ui->tableWidget_dem->rowCount(); ++row)
//     {
//         QStringList rowData;
//         for (int column = 0; column < ui->tableWidget_dem->columnCount(); ++column)
//         {
//             QTableWidgetItem *item = ui->tableWidget_dem->item(row, column);
//             rowData << (item ? item->text() : "");
//         }
//         out << rowData.join(",") << "\n";
//     }
//     file.close();
//     ui->statusbar->showMessage("Successfully save demand", 5000);
// }

// void QKDSim::readNetTable()
// {
//     // 读取节点信息
//     QString nodeNumString = ui->edit_node_num->text();
//     if (nodeNumString.isEmpty())
//     {
//         ui->statusbar->showMessage("Error: Missing data in node number");
//         return;
//     }
//     int nodeNum = nodeNumString.toInt();
//     net->SetNodeNum(nodeNum);
//     net->InitNodes(nodeNum);

//     // 逐行读取链路信息
//     for (int row = 0; row < ui->tableWidget_net->rowCount(); ++row)
//     {
//         LINKID linkId;
//         NODEID sourceId, sinkId;
//         RATE keyRate, bandWidth;
//         TIME proDelay, faultTime;
//         WEIGHT weight;

//         // 从表格的每一列读取数据
//         if (ui->tableWidget_net->item(row, 0) && ui->tableWidget_net->item(row, 1) && ui->tableWidget_net->item(row, 2) && ui->tableWidget_net->item(row, 3)
//                 && ui->tableWidget_net->item(row, 4) && ui->tableWidget_net->item(row, 5) && ui->tableWidget_net->item(row, 6))
//         {
//             // 转换为相应的数据类型
//             linkId = ui->tableWidget_net->item(row, 0)->text().toUInt();
//             sourceId = ui->tableWidget_net->item(row, 1)->text().toUInt();
//             sinkId = ui->tableWidget_net->item(row, 2)->text().toUInt();
//             keyRate = ui->tableWidget_net->item(row, 3)->text().toDouble();  // 假设 keyRate 是一个双精度浮点数
//             proDelay = ui->tableWidget_net->item(row, 4)->text().toDouble(); // 假设 proDelay 是一个双精度浮点数
//             bandWidth = ui->tableWidget_net->item(row, 5)->text().toDouble();
//             weight = ui->tableWidget_net->item(row, 6)->text().toDouble();
//             faultTime = ui->tableWidget_net->item(row, 7)->text() == "" ? -1 : ui->tableWidget_net->item(row, 7)->text().toDouble();    // 没有故障，则为-1

//             // 处理链路信息
//             CLink newLink;
//             newLink.SetLinkId(linkId);
//             newLink.SetSourceId(sourceId);
//             newLink.SetSinkId(sinkId);
//             newLink.SetQKDRate(keyRate);
//             newLink.SetLinkDelay(proDelay);
//             newLink.SetBandwidth(bandWidth);
//             newLink.SetWeight(weight);
//             newLink.SetFaultTime(faultTime);

//             net->m_vAllLinks.push_back(newLink);
//             net->m_mNodePairToLink[make_pair(sourceId, sinkId)] = linkId;
//             net->m_mNodePairToLink[make_pair(sinkId, sourceId)] = linkId;
//             net->InitKeyManagerOverLink(linkId);


//             net->m_vAllNodes[sourceId].m_lAdjNodes.push_back(sinkId);
//             net->m_vAllNodes[sinkId].m_lAdjNodes.push_back(sourceId);
//             net->InitKeyManagerOverLink(linkId);

//             /**********如何赋值**************/
//             int id_faultTime = 1000000 + linkId;    // m_mDemandArriveTime插入的value为1000000 + linkId     ？？可能有bug
//             if (faultTime > 0)
//                 net->m_mDemandArriveTime.insert(make_pair(faultTime, id_faultTime));   // 增加m_mDemandArriveTime视为增加故障点
//         }
//         else
//         {
//             ui->statusbar->showMessage("Error: Missing data in network table");
//         }
//     }
//     net->SetLinkNum(ui->tableWidget_net->rowCount() - 1); //第一行是link数量，需要rowCount()-1
// //    ui->statusbar->showMessage("Network data processed successfully", 5000);
// }

// void QKDSim::readDemTable()
// {
//     // 逐行读取需求信息
//     for (int row = 0; row < ui->tableWidget_dem->rowCount(); ++row)
//     {
//         DEMANDID demandId;
//         NODEID sourceId, sinkId;
//         VOLUME demandVolume;
//         TIME arriveTime;

//         // 从表格的每一列读取数据
//         if (ui->tableWidget_dem->item(row, 0) && ui->tableWidget_dem->item(row, 1) && ui->tableWidget_dem->item(row, 2)
//                 && ui->tableWidget_dem->item(row, 3) && ui->tableWidget_dem->item(row, 4))
//         {
//             // 转换为相应的数据类型
//             demandId = ui->tableWidget_dem->item(row, 0)->text().toUInt();
//             sourceId = ui->tableWidget_dem->item(row, 1)->text().toUInt();
//             sinkId = ui->tableWidget_dem->item(row, 2)->text().toUInt();
//             demandVolume = ui->tableWidget_dem->item(row, 3)->text().toDouble();  // 假设 demandVolume 是一个双精度浮点数
//             arriveTime = ui->tableWidget_dem->item(row, 4)->text().toDouble();  // 假设 arriveTime 是一个双精度浮点数

//             // 处理需求信息
//             CDemand newDemand;
//             newDemand.SetDemandId(demandId);
//             newDemand.SetSourceId(sourceId);
//             newDemand.SetSinkId(sinkId);
//             newDemand.SetDemandVolume(demandVolume);
//             newDemand.SetRemainingVolume(demandVolume);
//             newDemand.SetArriveTime(arriveTime);
//             newDemand.SetCompleteTime(INF); // 假设 INF 是一个定义好的常量，表示无限大
//             net->m_vAllDemands.push_back(newDemand);
//             net->m_vAllNodes[sourceId].m_mRelayVolume[demandId] = demandVolume;   //对m_mRelayVolume做初始化
//             net->m_mDemandArriveTime.insert(make_pair(arriveTime, demandId));   // 增加m_mDemandArriveTime
//         }
//         else
//         {
//             ui->statusbar->showMessage("Error: Missing data in demand table");
//         }
//     }


//     net->SetDemandNum(ui->tableWidget_dem->rowCount() - 1); //第一行是demand数量，需要rowCount()-1
// //    ui->statusbar->showMessage("Demand data processed successfully", 5000);
// }

// void QKDSim::showOutput()
// {
//     ui->edit_time->setText(QString::number(net->CurrentTime(), 'f', 2));

//     ui->tableWidget_out->clear();
//     ui->tableWidget_out->setRowCount(0);    // 清空表格
//     QStringList headers = {"需求ID", "当前节点", "下一节点", "下一跳链路", "可用密钥", "待传数据量", "已传完", "已故障"};     // 尚未确定
//     ui->tableWidget_out->setColumnCount(headers.size());
//     ui->tableWidget_out->setHorizontalHeaderLabels(headers);

//     // 遍历每个节点上正在传输的数据量
//     for (NODEID nodeId = 0; nodeId < net->GetNodeNum(); nodeId++)
//     {
//         for (auto demandIter = net->m_vAllNodes[nodeId].m_mRelayVolume.begin(); demandIter != net->m_vAllNodes[nodeId].m_mRelayVolume.end();)
//         {
//             if (net->m_vAllDemands[demandIter->first].GetRoutedFailed())
//             {
//                 demandIter = net->m_vAllNodes[nodeId].m_mRelayVolume.erase(demandIter);
//                 continue;
//             }
//             DEMANDID demandId = demandIter->first;
//             VOLUME relayVolume = demandIter->second;
//             bool isDelivered = net->m_vAllDemands[demandId].GetAllDelivered();
//             // 对于每个需求，从其路径中找到下一个要中继到的节点 nextNode
//             NODEID nextNode = net->m_vAllDemands[demandId].m_Path.m_mNextNode[nodeId];
//             // 找到当前节点和下一个节点之间的链路 minLink
//             LINKID minLink = net->m_mNodePairToLink[make_pair(nodeId, nextNode)];
//             VOLUME avaiableKeys = net->m_vAllLinks[minLink].GetAvaialbeKeys();
//             bool isRouteFailed = net->m_vAllDemands[demandId].GetRoutedFailed();

//             int newRow = ui->tableWidget_out->rowCount();
//             ui->tableWidget_out->insertRow(newRow);    // 末尾增加一行

//             ui->tableWidget_out->setItem(newRow, 0, new QTableWidgetItem(QString::number(demandId)));
//             ui->tableWidget_out->setItem(newRow, 1, new QTableWidgetItem(QString::number(nodeId)));
//             ui->tableWidget_out->setItem(newRow, 2, new QTableWidgetItem(QString::number(nextNode)));
//             ui->tableWidget_out->setItem(newRow, 3, new QTableWidgetItem(QString::number(minLink)));
//             ui->tableWidget_out->setItem(newRow, 4, new QTableWidgetItem(QString::number(avaiableKeys, 'f', 2)));
//             ui->tableWidget_out->setItem(newRow, 5, new QTableWidgetItem(QString::number(relayVolume, 'f', 2)));
//             ui->tableWidget_out->setItem(newRow, 6, new QTableWidgetItem(isDelivered ? "True" : "False"));
//             ui->tableWidget_out->setItem(newRow, 7, new QTableWidgetItem(isRouteFailed ? "True" : "False"));

//             demandIter++;
//         }
//     }
//     // 显示已传输完毕的数据
//     for (auto demandIter = net->m_vAllDemands.begin(); demandIter != net->m_vAllDemands.end(); demandIter++)
//     {
//         if (demandIter->GetAllDelivered())
//         {
//             NODEID nodeId = demandIter->GetSinkId();
//             DEMANDID demandId = demandIter->GetDemandId();
//             VOLUME relayVolume = 0;
//             bool isDelivered = demandIter->GetAllDelivered();
// //            NODEID nextNode = demandIter->GetSinkId();
// //            LINKID minLink = net->m_mNodePairToLink[make_pair(nodeId, nextNode)];
// //            VOLUME avaiableKeys = net->m_vAllLinks[minLink].GetAvaialbeKeys();
//             bool isRouteFailed = net->m_vAllDemands[demandId].GetRoutedFailed();

//             int newRow = ui->tableWidget_out->rowCount();
//             ui->tableWidget_out->insertRow(newRow);    // 末尾增加一行

//             ui->tableWidget_out->setItem(newRow, 0, new QTableWidgetItem(QString::number(demandId)));
//             ui->tableWidget_out->setItem(newRow, 1, new QTableWidgetItem(QString::number(nodeId)));
//             ui->tableWidget_out->setItem(newRow, 2, new QTableWidgetItem());
//             ui->tableWidget_out->setItem(newRow, 3, new QTableWidgetItem());
//             ui->tableWidget_out->setItem(newRow, 4, new QTableWidgetItem());
//             ui->tableWidget_out->setItem(newRow, 5, new QTableWidgetItem(QString::number(relayVolume, 'f', 2)));
//             ui->tableWidget_out->setItem(newRow, 6, new QTableWidgetItem(isDelivered ? "True" : "False"));
//             ui->tableWidget_out->setItem(newRow, 7, new QTableWidgetItem(isRouteFailed ? "True" : "False"));
//         }
//     }

//     ui->tableWidget_path->clear();
//     ui->tableWidget_path->setRowCount(0);    // 清空表格
//     QStringList headers_path = {"需求ID", "节点1", "节点2"};     // 至少首尾两个节点
//     ui->tableWidget_path->setColumnCount(headers_path.size());
//     ui->tableWidget_path->setHorizontalHeaderLabels(headers_path);

//     // 显示每个需求的路由的最短路径
//     for (auto demandIter = net->m_vAllDemands.begin(); demandIter != net->m_vAllDemands.end(); demandIter++)
//     {
//         if (demandIter->GetRoutedFailed())
//         {
//             DEMANDID demandId = demandIter->GetDemandId();
//             int newRow = ui->tableWidget_path->rowCount();
//             ui->tableWidget_path->insertRow(newRow);    // 末尾增加一行
//             ui->tableWidget_path->setItem(newRow, 0, new QTableWidgetItem(QString::number(demandId)));
//             // 都显示-1
//             ui->tableWidget_path->setItem(newRow, 1, new QTableWidgetItem("-1"));
//             ui->tableWidget_path->setItem(newRow, 2, new QTableWidgetItem("-1"));
//         }
//         else
//         {
//             DEMANDID demandId = demandIter->GetDemandId();
//             list<NODEID> node_path = net->m_vAllDemands[demandId].m_Path.m_lTraversedNodes;

//             // 如果当前行需要的列数大于表格当前列数，则扩展表格的列数
//             int currentColCount = static_cast<int>(node_path.size() + 1); // 因为第一列是demandId
//             if (currentColCount > ui->tableWidget_path->columnCount())
//             {
//                 int oldColCount = ui->tableWidget_path->columnCount();
//                 ui->tableWidget_path->setColumnCount(currentColCount);
//                 // 设置新的列头
//                 for (int col = oldColCount; col < currentColCount; ++col)
//                 {
//                     ui->tableWidget_path->setHorizontalHeaderItem(col, new QTableWidgetItem(QString("节点%1").arg(col)));
//                 }
//             }

//             int newRow = ui->tableWidget_path->rowCount();
//             ui->tableWidget_path->insertRow(newRow);    // 末尾增加一行
//             // 填充当前行的数据
//             ui->tableWidget_path->setItem(newRow, 0, new QTableWidgetItem(QString::number(demandId)));
//             int col = 1;  // 索引变量
//             for (auto it = node_path.begin(); it != node_path.end(); ++it, ++col)
//             {
//                 ui->tableWidget_path->setItem(newRow, col, new QTableWidgetItem(QString::number(*it)));
//             }
//         }
//     }
// }

// void QKDSim::on_bt_start_clicked()
// {
//     // 读取输入
//     net->Clear();
//     readNetTable();
//     readDemTable();
//     // 根据选中的按钮更改算法
//     if (ui->bt_route1->isChecked())
//         net->setShortestPath();
//     else
//         net->setKeyRateShortestPath();
//     if (ui->bt_schedule1->isChecked())
//         net->setMinimumRemainingTimeFirst();
//     else
//         net->setAverageKeyScheduling();

//     // 初始化
//     QFuture<void> future = QtConcurrent::run([this]()
//     {
//         this->net->InitRelayPath();
//         QMetaObject::invokeMethod(this, "showOutput");  // InitRelayPath 完成后显示输出

//     });
//     QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
//     connect(watcher, &QFutureWatcher<void>::started, loadingDialog, &QDialog::exec);
//     connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]()
//     {
//         loadingDialog->accept(); // 关闭加载对话框
//         watcher->deleteLater();  // 清理 watcher
//     });
//     watcher->setFuture(future);
// //    net->InitRelayPath();
// //    showOutput();
//     ui->statusbar->showMessage("All network and demand is init!");
// }

// void QKDSim::on_bt_begin_clicked()
// {
//     if (timer->isActive())
//     {
//         ui->bt_begin->setText("开始");
//         timer->stop();
//     }
//     else
//     {
//         ui->bt_begin->setText("暂停");
//         timer->start(1000);
//     }
// }

// void QKDSim::next_step()
// {
//     if (!net->AllDemandsDelivered())
//     {
//         TIME executeTime = net->OneTimeRelay();
//         net->MoveSimTime(executeTime);

// //        showOutput();
//         ui->statusbar->showMessage(QString("Now is %1 step").arg(net->CurrentStep()));
//     }
//     else
//     {
//         ui->statusbar->showMessage(QString("All demand has benn delivered, the end step is %1").arg(net->CurrentStep()));
//     }
// }

// void QKDSim::on_bt_next_clicked()
// {
//     QFuture<void> future = QtConcurrent::run([this]()
//     {
//         this->next_step();
//         QMetaObject::invokeMethod(this, "showOutput");
// //        showOutput();
//     });
//     QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
//     connect(watcher, &QFutureWatcher<void>::started, loadingDialog, &QDialog::exec);
//     connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]()
//     {
//         loadingDialog->accept(); // 关闭加载对话框
//         watcher->deleteLater();  // 清理 watcher
//     });
//     watcher->setFuture(future);
// //    next_step();
// }

// void QKDSim::on_bt_next10_clicked()
// {
//     QFuture<void> future = QtConcurrent::run([this]()
//     {
//         for (int i = 0; i < 10; i++)
//             next_step();
//         QMetaObject::invokeMethod(this, "showOutput");
// //        showOutput();
//     });
//     QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
//     connect(watcher, &QFutureWatcher<void>::started, loadingDialog, &QDialog::exec);
//     connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]()
//     {
//         loadingDialog->accept(); // 关闭加载对话框
//         watcher->deleteLater();  // 清理 watcher
//     });
//     watcher->setFuture(future);
// //    for (int i = 0; i < 10; i++)
// //        next_step();
// }

// void QKDSim::showNodeGraph()
// {
//     NODEID centerNodeId = ui->edit_show_node->text().toInt(); // 中心节点
//     if (centerNodeId >= net->GetNodeNum())
//     {
//         QMessageBox::critical(nullptr, "错误", "没有这个节点！");
//         return;
//     }

//     scene->clear();
//     int WIDTH = ui->graph_node->size().width() - 10;
//     int HEIGHT = ui->graph_node->size().height() - 10;
//     int NODE_SIZE = min(WIDTH, HEIGHT) / 10;
//     qreal RADIUS = min(WIDTH, HEIGHT) / 4.2;

//     scene->setSceneRect(0, 0, WIDTH, HEIGHT);  // 场景大小

//     unordered_set<NODEID> nodeShow;
//     unordered_set<LINKID> linkShow;
//     unordered_map<NODEID, pair<int, int>> loc;    // 节点位置

//     // 中心节点
//     scene->addEllipse(WIDTH / 2 - NODE_SIZE / 2, HEIGHT / 2 - NODE_SIZE / 2, NODE_SIZE, NODE_SIZE, QPen(), QBrush(Qt::cyan));
//     QGraphicsTextItem* centerNodeText = scene->addText(QString::number(centerNodeId), QFont("Arial", 24));
//     centerNodeText->setPos(WIDTH / 2 - centerNodeText->boundingRect().width() / 2, HEIGHT / 2 - centerNodeText->boundingRect().height() / 2);
//     nodeShow.emplace(centerNodeId);
//     loc[centerNodeId] = make_pair(WIDTH / 2, HEIGHT / 2);

//     int node_layer = ui->box_layer->currentText().toInt();
//     unordered_set<NODEID> per1Nodes;   // 第一层节点
//     if (node_layer >= 1)    // 显示第一层（距离中心节点为1）的节点与链路
//     {
//         for (auto linkIter = net->m_vAllLinks.begin(); linkIter != net->m_vAllLinks.end(); linkIter++)
//         {
//             if (linkIter->GetSourceId() == centerNodeId)
//                 per1Nodes.emplace(linkIter->GetSinkId());
//             if (linkIter->GetSinkId() == centerNodeId)
//                 per1Nodes.emplace(linkIter->GetSourceId());
//         }

//         int numPer1Node = static_cast<int>(per1Nodes.size());
//         auto per1Node = per1Nodes.begin();
//         // 以中心节点为圆心的一个圆环，分布第一层节点
//         for (int i = 0; i < numPer1Node; i++, per1Node++)
//         {
//             qreal angle = 2 * M_PI * i / numPer1Node;  // 分割圆周
//             qreal x = WIDTH / 2 + RADIUS * cos(angle);
//             qreal y = HEIGHT / 2 + RADIUS * sin(angle);
//             scene->addEllipse(x - NODE_SIZE / 2, y - NODE_SIZE / 2, NODE_SIZE, NODE_SIZE, QPen(), QBrush(Qt::lightGray));
//             QGraphicsTextItem* peripheralNodeText = scene->addText(QString::number(*per1Node), QFont("Arial", 24)); // 给每个节点一个编号
//             peripheralNodeText->setPos(x - peripheralNodeText->boundingRect().width() / 2, y - peripheralNodeText->boundingRect().height() / 2);
//             // 位置
//             nodeShow.emplace(*per1Node);
//             loc[*per1Node] = make_pair(x, y);
//         }
//     }
//     unordered_set<NODEID> per2Nodes;   // 第二层节点
//     if (node_layer >= 2)    // 显示第二层（距离中心节点为2）的节点与链路
//     {
//         /************完成第二层节点的显示***************/
//         qreal RADIUS_LAYER2 = RADIUS * 2; // 第二层的半径比第一层大200
//         // 获取并显示第二层的节点
//         for (auto linkIter = net->m_vAllLinks.begin(); linkIter != net->m_vAllLinks.end(); ++linkIter)
//         {
//             NODEID sourceId = linkIter->GetSourceId();
//             NODEID sinkId = linkIter->GetSinkId();

//             // 当此link的sourceId在第一层，且sinkId不在nodeShow（不为中心节点且不在第一层）
//             if (auto conNode = per1Nodes.find(sourceId); conNode != per1Nodes.end() && nodeShow.find(sinkId) == nodeShow.end())
//                 per2Nodes.emplace(sinkId);
//             // 当此link的sinkId在第一层，且sourceId不在nodeShow（不为中心节点且不在第一层）
//             if (auto conNode = per1Nodes.find(sinkId); conNode != per1Nodes.end() && nodeShow.find(sourceId) == nodeShow.end())
//                 per2Nodes.emplace(sourceId);
//         }

//         int numPer2Node = static_cast<int>(per2Nodes.size());
//         auto per2Node = per2Nodes.begin();
//         // 以中心节点为圆心的一个圆环，分布第二层节点
//         for (int i = 0; i < numPer2Node; i++, per2Node++)
//         {
//             qreal angle = 2 * M_PI * i / numPer2Node + M_PI / 6; // 分割圆周 为了岔开，第二层起始点加30度
//             qreal x = WIDTH / 2 + RADIUS_LAYER2 * cos(angle);
//             qreal y = HEIGHT / 2 + RADIUS_LAYER2 * sin(angle);
//             scene->addEllipse(x - NODE_SIZE / 2, y - NODE_SIZE / 2, NODE_SIZE, NODE_SIZE, QPen(), QBrush(Qt::lightGray));
//             QGraphicsTextItem* peripheralNodeText = scene->addText(QString::number(*per2Node), QFont("Arial", 24)); // 给每个节点一个编号
//             peripheralNodeText->setPos(x - peripheralNodeText->boundingRect().width() / 2, y - peripheralNodeText->boundingRect().height() / 2);
//             // 位置
//             nodeShow.emplace(*per2Node);
//             loc[*per2Node] = make_pair(x, y);
//         }
//     }
//     // 所有显示节点的两两间的链路
//     for (auto i = nodeShow.begin(); i != nodeShow.end(); i++)
//     {
//         auto next = i;
//         for(auto j = ++next; j != nodeShow.end(); j++)
//         {
//             if(net->m_mNodePairToLink.find(make_pair(*i, *j)) != net->m_mNodePairToLink.end())
//             {
//                 qreal x1 = loc[*i].first;
//                 qreal y1 = loc[*i].second;
//                 qreal x2 = loc[*j].first;
//                 qreal y2 = loc[*j].second;
//                 scene->addLine(x1, y1, x2, y2, QPen(Qt::gray));
//                 LINKID linkId = net->m_mNodePairToLink[make_pair(*i, *j)];
//                 if (ui->check_show_linkid->isChecked() == true)
//                 {
//                     QGraphicsTextItem* lineText = scene->addText(QString::number(linkId), QFont("Arial", 16)); // 线上也显示编号
//                     lineText->setPos((x1 + x2) / 2 - lineText->boundingRect().width() / 2, (y1 + y2) / 2 - lineText->boundingRect().height() / 2);
//                 }
//                 linkShow.emplace(linkId);
//             }
//         }
//     }

//     ui->graph_node->show();

//     // 显示node
//     ui->tableWidget_node->clear();
//     ui->tableWidget_node->setRowCount(0);    // 清空表格
//     QStringList headers_node = {"需求ID", "节点ID", "下一跳链路", "待传数据量"};     // 尚未确定
//     ui->tableWidget_node->setColumnCount(headers_node.size());
//     ui->tableWidget_node->setHorizontalHeaderLabels(headers_node);
//     for (NODEID nodeId : nodeShow)
//     {
//         for (auto demandIter = net->m_vAllNodes[nodeId].m_mRelayVolume.begin(); demandIter != net->m_vAllNodes[nodeId].m_mRelayVolume.end(); demandIter++)
//         {
// //            if (net->m_vAllDemands[demandIter->first].GetRoutedFailed()||net->m_vAllDemands[demandIter->first].GetAllDelivered())
// //            {
// //                continue;
// //            }
//             DEMANDID demandId = demandIter->first;
//             VOLUME relayVolume = demandIter->second;
//             // 对于每个需求，从其路径中找到下一个要中继到的节点 nextNode
//             NODEID nextNode = net->m_vAllDemands[demandId].m_Path.m_mNextNode[nodeId];
//             // 找到当前节点和下一个节点之间的链路 minLink
//             LINKID minLink = net->m_mNodePairToLink[make_pair(nodeId, nextNode)];

//             int newRow = ui->tableWidget_node->rowCount();
//             ui->tableWidget_node->insertRow(newRow);    // 末尾增加一行

//             ui->tableWidget_node->setItem(newRow, 0, new QTableWidgetItem(QString::number(demandId)));
//             ui->tableWidget_node->setItem(newRow, 1, new QTableWidgetItem(QString::number(nodeId)));
//             ui->tableWidget_node->setItem(newRow, 2, new QTableWidgetItem(QString::number(minLink)));
//             ui->tableWidget_node->setItem(newRow, 3, new QTableWidgetItem(QString::number(relayVolume, 'f', 2)));
//         }
//     }
//     // 显示link
//     ui->tableWidget_link->clear();
//     ui->tableWidget_link->setRowCount(0);    // 清空表格
//     QStringList headers_link = {"链路ID", "节点1", "节点2", "可用密钥"};     // 尚未确定
//     ui->tableWidget_link->setColumnCount(headers_link.size());
//     ui->tableWidget_link->setHorizontalHeaderLabels(headers_link);
//     for (LINKID linkId : linkShow)
//     {
//         NODEID nodeId = net->m_vAllLinks[linkId].GetSourceId();
//         NODEID nextNode = net->m_vAllLinks[linkId].GetSinkId();
//         VOLUME avaiableKeys = net->m_vAllLinks[linkId].GetAvaialbeKeys();

//         int newRow = ui->tableWidget_link->rowCount();
//         ui->tableWidget_link->insertRow(newRow);    // 末尾增加一行

//         ui->tableWidget_link->setItem(newRow, 0, new QTableWidgetItem(QString::number(linkId)));
//         ui->tableWidget_link->setItem(newRow, 1, new QTableWidgetItem(QString::number(nodeId)));
//         ui->tableWidget_link->setItem(newRow, 2, new QTableWidgetItem(QString::number(nextNode)));
//         ui->tableWidget_link->setItem(newRow, 3, new QTableWidgetItem(QString::number(avaiableKeys, 'f', 2)));
//     }
// }

// void QKDSim::on_bt_show_node_clicked()
// {
//     showNodeGraph();
// }

// // 自动加减行
// void tableWidget_cellChanged(int row, int column, QTableWidget* tableWidget)

// {
//     // 最后一行添加数据，则再加一个空行
//     int rowCount = tableWidget->rowCount();
//     if (row == rowCount - 1)
//     {
//         tableWidget->insertRow(rowCount);
//     }

//     // 某一行删除数据，查看此行是否为空行，并删除
//     QTableWidgetItem* item = tableWidget->item(row, column);
//     if (item != nullptr && item->text().isEmpty())
//     {
//         // 如果单元格为空，检查整行是否也为空
//         bool isRowEmpty = true;
//         for (int j = 0; j < tableWidget->columnCount(); ++j)
//         {
//             QTableWidgetItem* cell = tableWidget->item(row, j);
//             if (cell != nullptr && !cell->text().isEmpty())
//             {
//                 isRowEmpty = false;
//                 break;
//             }
//         }
//         if (isRowEmpty)
//         {
//             tableWidget->removeRow(row);  // 如果整行为空，则删除该行
//         }
//     }
// }

// void QKDSim::on_tableWidget_net_cellChanged(int row, int column)
// {
//     tableWidget_cellChanged(row, column, ui->tableWidget_net);
// }

// void QKDSim::on_tableWidget_dem_cellChanged(int row, int column)
// {
//     tableWidget_cellChanged(row, column, ui->tableWidget_dem);
// }
