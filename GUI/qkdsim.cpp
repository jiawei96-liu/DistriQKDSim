
#include "qkdsim.h"
#include "ui_qkdsim.h"
#include <QtConcurrent>

QKDSim::QKDSim(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::QKDSim)
{
    ui->setupUi(this);
    ui->tableWidget_in->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 表格列宽自动伸缩
    ui->tableWidget_in->verticalHeader()->setDefaultSectionSize(20);
    //    ui->tableWidget_dem->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //    ui->tableWidget_dem->verticalHeader()->setDefaultSectionSize(20);
    ui->tableWidget_path->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_path->verticalHeader()->setDefaultSectionSize(20);
    ui->tableWidget_out->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_out->verticalHeader()->setDefaultSectionSize(20);
    ui->tableWidget_link->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_link->verticalHeader()->setDefaultSectionSize(20);
    ui->tableWidget_node->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_node->verticalHeader()->setDefaultSectionSize(20);

    ui->tableWidget_path->horizontalHeader()->setVisible(false);

    // 读取csv文件


    // loadCSV("../../Input/network.csv", Network);
    // loadCSV("../../Input/demand.csv", Demand);

    // loadCSV("../../Input/network(1000) - 副本.csv", Network);
    // loadCSV("../../Input/demand.csv", Demand);


    // loadCSV("../../Input/10规模/network.csv", Network);
    // loadCSV("../../Input/10规模/demand.csv", Demand);

    loadCSV("../../Input/100规模/network.csv", Network);
    loadCSV("../../Input/100规模/demand.csv", Demand);

    // loadCSV("../../Input/network(500).csv", Network);
    // loadCSV("../../Input/demand(500).csv", Demand);

    // loadCSV("../../Input/1000规模/network.csv", Network);
    // loadCSV("../../Input/1000规模/demand.csv", Demand);

    // 显示demand
    showCSV(Network);

    timer = new QTimer(this);

    Connections();
    // 设置CNetwork
    net = new CNetwork();
    // 设置拓扑图
    scene = new QGraphicsScene(this);
    ui->graph_node->setScene(scene);
    // 进度条界面
    loadingDialog = new LoadingDialog(this);
}

QKDSim::~QKDSim()
{
    delete net;
    delete ui;
}

void QKDSim::Connections()
{
    // 关于QT
    connect(ui->action_qt, &QAction::triggered, [this]
    { QMessageBox::aboutQt(this); });

    // 关闭程序
    connect(ui->action_exit, &QAction::triggered, this, &QWidget::close);

    // 保存打开文件（接收内容显示框）
    connect(ui->action_open_net, &QAction::triggered, this, &QKDSim::open_net);
    connect(ui->action_open_dem, &QAction::triggered, this, &QKDSim::open_dem);
    connect(ui->action_save_net, &QAction::triggered, this, &QKDSim::save_net);
    connect(ui->action_save_dem, &QAction::triggered, this, &QKDSim::save_dem);

    // 定时器
    //    connect(timer, &QTimer::timeout, this, &QKDSim::next_step);
    connect(timer, &QTimer::timeout, this, [this]()
    {
//        QFuture<void> future = QtConcurrent::run([this]()
//        {
//            QMetaObject::invokeMethod(timer, "stop");  // 定时器阻塞期间不运行
//            this->next_step();
//            QMetaObject::invokeMethod(this, "showOutput");
//            QMetaObject::invokeMethod(timer, "start", Q_ARG(int, 1000));
//        });
//        QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
//        connect(watcher, &QFutureWatcher<void>::started, loadingDialog, &QDialog::exec);
//        connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]()
//        {
//            loadingDialog->accept(); // 关闭加载对话框
//            watcher->deleteLater();  // 清理 watcher
//        });
//        watcher->setFuture(future);
        timer->stop();
        this->next_step();
        showOutput();
        showNodeGraph();
        timer->start(1000);
    });

    // 进度条界面
    //    connect(this, &QKDSim::progressChanged, progressBar, &ProgressBar::updateProgress);
}

void QKDSim::open_net()
{
    // 加载文件至接收内容显示框
    QString filename = QFileDialog::getOpenFileName(this, tr("打开网络拓扑文件"));
    loadCSV(filename, Network);
    //    ui->tabWidget->setCurrentIndex(0);
    ui->box_input->setCurrentIndex(0);
    showCSV(Network);
}

void QKDSim::open_dem()
{
    // 加载文件至接收内容显示框
    QString filename = QFileDialog::getOpenFileName(this, tr("打开需求文件"));
    loadCSV(filename, Demand);
    //    ui->tabWidget->setCurrentIndex(1);
    ui->box_input->setCurrentIndex(1);
    showCSV(Demand);
}

void QKDSim::save_net()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("保存网络拓扑"), QDir::currentPath(), tr("CSV Files (*.csv);;All Files (*.*)"));
    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
    {
        ui->statusbar->showMessage("Save file failed", 5000);
        return;
    }

    QTextStream out(&file);
    out << ui->edit_node_num->text().toInt() << "\n"; // 第一行存nodenum
    //    for (int row = 0; row < ui->tableWidget_in->rowCount(); ++row)
    //    {
    //        QStringList rowData;
    //        for (int column = 0; column < ui->tableWidget_in->columnCount(); ++column)
    //        {
    //            QTableWidgetItem *item = ui->tableWidget_in->item(row, column);
    //            rowData << (item ? item->text() : "");
    //        }
    //        out << rowData.join(",") << "\n";
    //    }
    for (auto &net : network)
    {
        QStringList rowData;
        rowData << QString::number(std::get<0>(net));
        rowData << QString::number(std::get<1>(net));
        rowData << QString::number(std::get<2>(net));
        rowData << QString::number(std::get<3>(net), 'f', 2);
        rowData << QString::number(std::get<4>(net), 'f', 2);
        rowData << QString::number(std::get<5>(net), 'f', 2);
        rowData << QString::number(std::get<6>(net), 'f', 2);
        rowData << QString::number(std::get<7>(net), 'f', 2);

        out << rowData.join(",") << "\n";
    }
    file.close();
    ui->box_input->setCurrentIndex(0);
    showCSV(Network);
    ui->statusbar->showMessage("Successfully save network", 5000);
}

void QKDSim::save_dem()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("保存网络需求"), QDir::currentPath(), tr("CSV Files (*.csv);;All Files (*.*)"));
    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
    {
        ui->statusbar->showMessage("Save file failed", 5000);
        return;
    }

    QTextStream out(&file);
    //    for (int row = 0; row < ui->tableWidget_dem->rowCount(); ++row)
    //    {
    //        QStringList rowData;
    //        for (int column = 0; column < ui->tableWidget_dem->columnCount(); ++column)
    //        {
    //            QTableWidgetItem *item = ui->tableWidget_dem->item(row, column);
    //            rowData << (item ? item->text() : "");
    //        }
    //        out << rowData.join(",") << "\n";
    //    }
    for (auto &dem : demand)
    {
        QStringList rowData;
        rowData << QString::number(std::get<0>(dem));
        rowData << QString::number(std::get<1>(dem));
        rowData << QString::number(std::get<2>(dem));
        rowData << QString::number(std::get<3>(dem), 'f', 2);
        rowData << QString::number(std::get<4>(dem), 'f', 2);

        out << rowData.join(",") << "\n";
    }
    file.close();
    ui->box_input->setCurrentIndex(1);
    showCSV(Demand);
    ui->statusbar->showMessage("Successfully save demand", 5000);
}

void QKDSim::on_bt_start_clicked()
{
    // 读取输入
    net->Clear(); // 重置

    //    readNetTable();
    //    readDemTable();
    if (ui->box_input->currentIndex() == 0)
        readTable(Network);
    else if (ui->box_input->currentIndex() == 1)
        readTable(Demand);
    readCSV(Network);
    readCSV(Demand);

    // 根据选中的按钮更改算法
    if (ui->bt_route1->isChecked())
        net->setShortestPath();
    else
        // net->setKeyRateShortestPath();
        net->setKeyRateShortestPathWithBinHeap();
    if (ui->bt_schedule1->isChecked())
        net->setMinimumRemainingTimeFirst();
    else
        net->setAverageKeyScheduling();

    // 初始化
    //    QFuture<void> future = QtConcurrent::run([this]()
    //    {
    //        this->net->InitRelayPath();
    //        QMetaObject::invokeMethod(this, "showOutput");  // InitRelayPath 完成后显示输出
    //        QMetaObject::invokeMethod(ui->statusbar, "showMessage",
    //                                  Q_ARG(QString, QString("%1 links and %2 demands are init").arg(net->GetLinkNum()).arg(net->GetDemandNum())));
    //    });
    //    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    //    connect(watcher, &QFutureWatcher<void>::started, loadingDialog, &QDialog::exec);
    //    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]()
    //    {
    //        loadingDialog->accept(); // 关闭加载对话框
    //        watcher->deleteLater();  // 清理 watcher
    //    });
    //    watcher->setFuture(future);
    net->InitRelayPath();
    net->InitLinkDemand();

    showOutput();
    ui->statusbar->showMessage(QString("%1 links and %2 demands are init").arg(net->GetLinkNum()).arg(net->GetDemandNum()));
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

// net执行下一步操作
void QKDSim::next_step()
{
    if (!net->AllDemandsDelivered())
    {
        TIME executeTime = net->OneTimeRelay();
        net->MoveSimTime(executeTime);

        //        showOutput();
        ui->statusbar->showMessage(QString("step %1").arg(net->CurrentStep()));
    }
    else
    {
        ui->statusbar->showMessage(QString("All demand have been delivered, the end step is %1").arg(net->CurrentStep()));
    }
}

void QKDSim::on_bt_next_clicked()
{
    //    QFuture<void> future = QtConcurrent::run([this]()
    //    {
    //        this->next_step();
    //        QMetaObject::invokeMethod(this, "showOutput");
    //    });
    //    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    //    connect(watcher, &QFutureWatcher<void>::started, loadingDialog, &QDialog::exec);
    //    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]()
    //    {
    //        loadingDialog->accept(); // 关闭加载对话框
    //        watcher->deleteLater();  // 清理 watcher
    //    });
    //    watcher->setFuture(future);
    next_step();
    showOutput();
}

void QKDSim::on_bt_next10_clicked()
{
    //    QFuture<void> future = QtConcurrent::run([this]()
    //    {
    //        for (int i = 0; i < 10; i++)
    //            next_step();
    //        QMetaObject::invokeMethod(this, "showOutput");

    //    });
    //    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    //    connect(watcher, &QFutureWatcher<void>::started, loadingDialog, &QDialog::exec);
    //    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]()
    //    {
    //        loadingDialog->accept(); // 关闭加载对话框
    //        watcher->deleteLater();  // 清理 watcher
    //    });
    //    watcher->setFuture(future);
    for (int i = 0; i < 10; i++)
        next_step();
    showOutput();
}

void QKDSim::on_bt_show_node_clicked()
{
    showNodeGraph();
}

// 表格自动加减行
void tableWidget_cellChanged(int row, int column, QTableWidget *tableWidget)

{
    // 最后一行添加数据，则再加一个空行
    int rowCount = tableWidget->rowCount();
    if (row == rowCount - 1)
    {
        tableWidget->insertRow(rowCount);
    }

    // 某一行删除数据，查看此行是否为空行，并删除
    QTableWidgetItem *item = tableWidget->item(row, column);
    if (item != nullptr && item->text().isEmpty())
    {
        // 如果单元格为空，检查整行是否也为空
        bool isRowEmpty = true;
        for (int j = 0; j < tableWidget->columnCount(); ++j)
        {
            QTableWidgetItem *cell = tableWidget->item(row, j);
            if (cell != nullptr && !cell->text().isEmpty())
            {
                isRowEmpty = false;
                break;
            }
        }
        if (isRowEmpty)
        {
            tableWidget->removeRow(row); // 如果整行为空，则删除该行
        }
    }
}

// void QKDSim::on_tableWidget_dem_cellChanged(int row, int column)
//{
//     tableWidget_cellChanged(row, column, ui->tableWidget_dem);
// }

void QKDSim::on_tableWidget_in_cellChanged(int row, int column)
{
    tableWidget_cellChanged(row, column, ui->tableWidget_in);
}

// 切换表格显示内容
void QKDSim::on_box_input_currentIndexChanged(int index)
{
    switch (index)
    {
    case 0:
        readTable(Demand);
        showCSV(Network);
        break;
    case 1:
        readTable(Network);
        showCSV(Demand);
        break;
    default:
        break;
    }
}
