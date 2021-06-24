﻿#include <cmath>
#include<QThread>
#include<QFileDialog>
#include <QSerialPortInfo>
#include <QStandardPaths>
#include <QMessageBox>

//#include <qwt_polar_curve.h>
//#include <qwt_series_data.h>
//#include <qwt_polar_grid.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "rotatorprotocol.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , telnet(this)
{
    status_n9918a = "disconnected";
    status_rotator = "disconnected";
    ui->setupUi(this);
    connect(&telnet, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(on_state_changed(QAbstractSocket::SocketState)));
    connect(&telnet, SIGNAL(newData(const char*,int)), this, SLOT(add_n9918a_log(const char*,int)));

    timerRead = new QTimer(this);
    timerRead->setInterval(100);
    connect(timerRead, SIGNAL(timeout()), this, SLOT(com_read_data()));

    refresh_cmd_port_list();
    xaxis = new QVector<double>();

    // 添加方向图控件
//    d_plot = new Plot( this );
//    ui->verticalLayout_5->addWidget(d_plot);
////    d_plot->showGrid(false);

//    //
//    plot.setPlotBackground (QBrush( Qt::red, Qt::SolidPattern ));
//    plot.setScale( QwtPolar::ScaleAzimuth, 0, 360, 30);
//    plot.setScale( QwtPolar::ScaleRadius, 0, 4, 2);

//    QwtPolarGrid grid;
//    grid.setFont (QFont("Times", 12, QFont::Bold));
//    grid.setPen ( QPen(Qt::blue, 1, Qt::DashDotLine) );
//    //grid.setAxisFont (QwtPolar::AxisLeft,  QFont("Times", 6));
//    grid.setAxisPen ( QwtPolar::AxisAzimuth, QPen(Qt::black, 1) );
//    //grid.setAxisPen ( QwtPolar::AxisLeft, QPen(Qt::black, 1) );
//    grid.showMinorGrid (QwtPolar::AxisLeft, true);
//    grid.showMinorGrid (QwtPolar::AxisRight, true);
//    grid.showMinorGrid (QwtPolar::AxisTop, true);
//    grid.showMinorGrid (QwtPolar::AxisBottom, true);
//    grid.showGrid (QwtPolar::AxisAzimuth, true);
//    grid.showGrid (QwtPolar::AxisLeft, true);
//    grid.showGrid (QwtPolar::AxisRight, true);
//    grid.showGrid (QwtPolar::AxisTop, true);
//    grid.showGrid (QwtPolar::AxisBottom, true);
//    grid.attach(&plot);
//    plot.resize(800,600);
//    plot.show();
    c = new QChart();
    pattern_data = new QSplineSeries();
    chart = new QPolarChart();
}

MainWindow::~MainWindow()
{
    telnet.disconnectFromHost();
    com->close();
    delete ui;
}

void MainWindow::on_state_changed(QAbstractSocket::SocketState s)
{
    switch( s )
        {
        case QAbstractSocket::UnconnectedState:
            set_status_text("disconnected", "");
            ui->lan_connect->setText(QStringLiteral("1-连接"));
            ui->start_test->setEnabled(false);
            ui->ip_addr->setEnabled(true);
            break;
        case QAbstractSocket::HostLookupState:
            set_status_text("connecting", "");
            ui->lan_connect->setText(QStringLiteral("断开"));
            break;
        case QAbstractSocket::ConnectingState:
            set_status_text("connecting", "");
            ui->lan_connect->setText(QStringLiteral("断开"));
            break;
        case QAbstractSocket::ConnectedState:
            set_status_text("connected", "");
            ui->lan_connect->setText(QStringLiteral("断开"));
            ui->ip_addr->setEnabled(false);
            break;
        case QAbstractSocket::BoundState:
            set_status_text("connected", "");
            ui->lan_connect->setText(QStringLiteral("断开"));
            break;
        case QAbstractSocket::ListeningState:
            set_status_text("connected", "");
            ui->lan_connect->setText(QStringLiteral("断开"));
            break;
        case QAbstractSocket::ClosingState:
            set_status_text("disconnected", "");
            ui->lan_connect->setText(QStringLiteral("断开"));
            break;
    }
}

void MainWindow::com_read_data()
{
    if (com->bytesAvailable() <= 0) {
        return;
    }
    QByteArray data = com->readAll();
    int dataLen = data.length();
    if (dataLen <= 0) {
        return;
    }
    int header = data.indexOf(0xaa);
    if (header == 0){
        // 发现帧头, 解析角度
        // pitch
        char pitch[2] = {data[8], data[7]};
        QByteArray pitch_a = QByteArray(pitch, 2);
        QDataStream dataStream(pitch_a);
        qint16 pitch_int;
        dataStream >> pitch_int;
        ui->current_pitch->setText(QString::number(((double)pitch_int)/100));
        // azimuth
        char azimuth[2] = {data[6], data[5]};
        QByteArray azimuth_a = QByteArray(azimuth, 2);

        QDataStream dataStream_azimuth(azimuth_a);
        quint16 azimuth_int;
        dataStream_azimuth >> azimuth_int;
        ui->current_azimuth->setText(QString::number(((double)azimuth_int)/100));
    }
}

void MainWindow::set_status_text(const QString &n9918a, const QString &rotator)
{
    if (n9918a.length()!=0){
        status_n9918a = n9918a;
    }

    if (rotator.length()!=0){
        status_rotator = rotator;
    }

    ui->statusbar->showMessage(QString("N9918A: %1 | Rotator: %2").arg(status_n9918a).arg(status_rotator));

    // 改变 开始按钮状态
    if (telnet.isConnected() && comOk){
        ui->start_test->setEnabled(true);
    }else{
        ui->start_test->setEnabled(false);
    }
}


void MainWindow::on_lan_connect_clicked()
{
    if (! telnet.isConnected()){
        telnet.connectToHost(ui->ip_addr->text(), 5025);
        ui->lan_connect->setText(QStringLiteral("断开"));
    }else{
        telnet.disconnectFromHost();
        ui->lan_connect->setText(QStringLiteral("1-连接"));
    }

}

void MainWindow::add_n9918a_log(const char *msg, int count)
{
    last_9918_anser = QByteArray(msg, count);
    ui->n9918_log->insertPlainText(last_9918_anser);
    ui->n9918_log->insertPlainText("\n");
    cmd_lock = false;

}

void MainWindow::init_n9918a()
{
    send_cmd_9918a("*CLS");
    send_cmd_9918a("*IDN?");
    send_cmd_9918a("SYST:PRES;*OPC?");
    send_cmd_9918a("INST:SEL 'SA';*OPC?");
    send_cmd_9918a(QString("SENS:SWE:POIN %1").arg(ui->sample_points->text()));
    send_cmd_9918a(QString("SENS:FREQ:START %1").arg(ui->start_freq->text()));
    send_cmd_9918a(QString("SENS:FREQ:STOP %1").arg(ui->stop_freq->text()));
}



void MainWindow::send_cmd_9918a(const QString &cmd)
{
    telnet.sendData(cmd.toUtf8());
    telnet.sendData("\n");
    ui->n9918_log->insertPlainText(QString("[CMD] %1\n").arg(QString(cmd.toUtf8())));
    // 当处理查询语句之后应该等待返回消息再发送下一条
    if (cmd.indexOf('?') != -1){
        cmd_lock = true;
        int timeout = 100;
        while(cmd_lock){
            QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
            timeout --;
            if (timeout == 0){
                ui->n9918_log->insertPlainText("Timeout!\n");
                break;
            }
            QThread::msleep(50);
        }
    }
}

void MainWindow::measure_power()
{
    send_cmd_9918a("INIT:CONT OFF;*OPC?");
    send_cmd_9918a("INIT:IMM;*OPC?");
    send_cmd_9918a("TRACE:DATA?");
    QStringList list = last_9918_anser.split(",");
    double max = -1e100;
    int max_index = -1;
    double min = 1e100;
    int index = -1;

    QLineSeries *lineseries = new QLineSeries();  //频谱的数据集

    foreach(QString num, list){
        index ++;
        double temp = num.toDouble();
        lineseries->append(xaxis->at(index), temp);
        if (max < temp){
            max = temp;
            max_index = index;
        }
        if (min > temp){
            min = temp;
        }

    }
    ui->power_max->setText(QString::number(max));
    ui->power_min->setText(QString::number(min));
    ui->peek_freq->setText(QString::number(max_index));

    // 绘制当前频谱
    c = new QChart();
    c->legend()->hide();
    c->addSeries(lineseries);
    c->createDefaultAxes();
    ui->spectrum->setChart(c);

    // 添加测量点到方向图
    pattern_data->append(ui->current_azimuth->text().toDouble(), max);

    chart = new QPolarChart();
    chart->legend()->hide();
    chart->addSeries(pattern_data);

    chart->createDefaultAxes();
    QList<QAbstractAxis *> axisList = chart->axes();
    axisList.at(0)->setRange(0,360);

    ui->pattern->setChart(chart);

}

void MainWindow::freq_linespace()
{
    if(xaxis->length()!=0){
        xaxis->clear();
    }

    double start_freq = ui->start_freq->text().toDouble();
    double stop_freq = ui->stop_freq->text().toDouble();
    int sample_points = ui->sample_points->text().toInt();
    xaxis = new QVector<double>();
    double step_freq = (stop_freq - start_freq)/sample_points;
    for (int i=0; i<sample_points; i++) {
        xaxis->push_back(start_freq + i*step_freq);
    }
}

void MainWindow::refresh_cmd_port_list()
{
    ui->cb_com_port->clear();
    // 列出可用串口
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->cb_com_port->addItem(info.portName());
    }


}

void MainWindow::send_cmd_rotator(const QByteArray &cmd)
{
    if (comOk){
        com->write(cmd);
    }
}

void MainWindow::turn_rotator(double azimuth)
{
    RotatorProtocol rp;
    rp.set_target_angle((int)(azimuth*100), 0, AZIMUTH);
    send_cmd_rotator(rp.get_bitstring());
    // wait rotator position
    int timeout = 100;
    double delta = azimuth - ui->current_azimuth->text().toDouble();

    while(!(-0.1 < delta && delta < 0.1)){
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        delta = azimuth - ui->current_azimuth->text().toDouble();
        if(kill_process){
            return;
        }
        timeout --;
        if (timeout == 0){
            ui->rotator_log->insertPlainText("Timeout!\n");
            break;
        }
        QThread::msleep(50);
    }
    ui->rotator_log->insertPlainText(QString("turn to the postion! %1\n").arg(ui->current_azimuth->text()));
}

QString MainWindow::select_save_file()
{
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this,
            tr("Save file"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), tr("CSV (*.csv)"));

    if (!fileName.isNull())
    {
        //fileName是文件名

        return fileName;
    }else{
        //点的是取消

        return fileName;
    }
}

void MainWindow::set_n9918_config_enable(bool enable)
{
    if (enable){
        ui->start_freq->setEnabled(true);
        ui->stop_freq->setEnabled(true);
        ui->sample_points->setEnabled(true);
    }else{
        ui->start_freq->setEnabled(false);
        ui->stop_freq->setEnabled(false);
        ui->sample_points->setEnabled(false);
    }
}

void MainWindow::set_rotator_config_enable(bool enable)
{
    if (enable){
        ui->start_azimuth->setEnabled(true);
        ui->stop_azimuth->setEnabled(true);
        ui->step_azimuth->setEnabled(true);
        ui->pitch->setEnabled(true);
    }else{
        ui->start_azimuth->setEnabled(false);
        ui->stop_azimuth->setEnabled(false);
        ui->step_azimuth->setEnabled(false);
        ui->pitch->setEnabled(false);
    }
}

void MainWindow::on_process_enable(bool enable)
{
    if (enable){
        // 禁用
        on_process = true;
        set_n9918_config_enable(false);
        set_rotator_config_enable(false);
        ui->start_test->setEnabled(false);
        ui->com_connect->setEnabled(false);
        ui->lan_connect->setEnabled(false);
        ui->set_pitch->setEnabled(false);
        ui->pb_set_azimuth->setEnabled(false);
        ui->le_azimuth->setEnabled(false);
    }else {
        // 启用
        on_process = false;
        ui->start_test->setEnabled(true);
        ui->com_connect->setEnabled(true);
        ui->lan_connect->setEnabled(true);
        ui->set_pitch->setEnabled(true);
        ui->pb_set_azimuth->setEnabled(true);
        ui->le_azimuth->setEnabled(true);
        set_n9918_config_enable(true);
        set_rotator_config_enable(true);
    }
}

void MainWindow::on_start_test_clicked()
{
    on_process_enable(true);

    // 选择文件保存路径
    QString filename = select_save_file();
    if(filename.length()==0){
        // 用户取消选择
        on_process_enable(false);
        return;
    }
    QFile file(filename);
    //判断文件是否存在
    if(file.exists()){
        QFile fileTemp(filename);
        fileTemp.remove();
    }else{
    }
    //已读写方式打开文件，
    //如果文件不存在会自动创建文件
    if(!file.open(QIODevice::ReadWrite)){
        QMessageBox messageBox(QMessageBox::Warning,
                               QStringLiteral("打开失败"), QStringLiteral("打开文件失败，请重新开始"),
                               QMessageBox::Yes , NULL);
        messageBox.exec();
        on_process_enable(false);
        return;
    }else{
    }
    // 写入表头
    QString content("set pitch, current pitch, set azimuth, current azimuth, data(dBm)\n");
    file.write(content.toUtf8(),content.length());

    ui->progressBar->setValue(0);
    init_n9918a();
    freq_linespace();
    QSplineSeries* old_data = pattern_data;

    pattern_data = new QSplineSeries();
    old_data->deleteLater();                  // 删掉旧的方向图数据

    double start_azimuth = ui->start_azimuth->text().toDouble();
    double stop_azimuth = ui->stop_azimuth->text().toDouble();
    double step_azimuth = ui->step_azimuth->text().toDouble();
    int movement = (int)((stop_azimuth-start_azimuth)/step_azimuth);
    int move_count = 0;
    double set_azimuth = start_azimuth;
    for (; move_count<movement; move_count++ ) {
        ui->progressBar->setValue((int)(((double)move_count)/movement*100));
        turn_rotator(set_azimuth);
        // Debug
        ui->current_azimuth->setText(QString::number(set_azimuth));
        if(kill_process){
            kill_process = false;
            on_process_enable(false);
            return;
        }

        // save data to csv
        measure_power();
        content = QString("%1, %2, %3, %4, %5\n").arg(ui->pitch->text()).arg(ui->current_pitch->text()).arg(set_azimuth).arg(ui->current_azimuth->text()).arg(last_9918_anser);
        file.write(content.toUtf8(),content.length());

        set_azimuth += step_azimuth;
    }
    if (set_azimuth != stop_azimuth){
        set_azimuth = stop_azimuth;
        turn_rotator(set_azimuth);
        // save data to csv
        measure_power();
        content = QString("%1, %2, %3, %4, %5\n").arg(ui->pitch->text()).arg(ui->current_pitch->text()).arg(set_azimuth).arg(ui->current_azimuth->text()).arg(last_9918_anser);
        file.write(content.toUtf8(),content.length());
    }
    file.close();

    ui->progressBar->setValue(100);
    on_process_enable(false);
}


void MainWindow::on_n9918_log_textChanged()
{
    ui->n9918_log->moveCursor(QTextCursor::End);
}


void MainWindow::on_com_connect_clicked()
{
    if (!comOk){
        // 初始化串口
        com = new QextSerialPort(ui->cb_com_port->currentText(), QextSerialPort::Polling);
        comOk = com->open(QIODevice::ReadWrite);
        if (comOk) {
            //清空缓冲区
            com->flush();
            //设置波特率
            com->setBaudRate(BAUD115200);
            //设置数据位
            com->setDataBits(DATA_8);
            //设置校验位
            com->setParity(PAR_NONE);
            //设置停止位
            com->setStopBits(STOP_1);
            com->setFlowControl(FLOW_OFF);
            com->setTimeout(10);
            ui->com_connect->setText(QStringLiteral("断开"));
            set_status_text("", "connected");
            ui->cb_com_port->setEnabled(false);
            ui->btn_refresh_com->setEnabled(false);

            //读取数据
            timerRead->start();
            ui->set_pitch->setEnabled(true);
            ui->pb_set_azimuth->setEnabled(true);
        }
    }else{
        // 断开串口
        timerRead->stop();
        comOk = false;
        com->close();
        com->deleteLater();
        ui->set_pitch->setEnabled(false);
        ui->pb_set_azimuth->setEnabled(false);
        ui->start_test->setEnabled(false);
        ui->cb_com_port->setEnabled(true);
        ui->btn_refresh_com->setEnabled(true);
        ui->com_connect->setText(QStringLiteral("2-连接"));
        set_status_text("", "disconnected");
    }
}




void MainWindow::on_set_pitch_clicked()
{
    RotatorProtocol rp;
    double pitch = ui->pitch->text().toDouble();
    rp.set_target_angle(0, (int)(pitch*100), PITCH);
    send_cmd_rotator(rp.get_bitstring());
}



void MainWindow::on_rotator_log_textChanged()
{
    ui->rotator_log->moveCursor(QTextCursor::End);
}


void MainWindow::on_stop_test_clicked()
{
    kill_process = true;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    //当文档内容被修改时.
    if (!on_process){
        event->accept();
    }else{
        event->ignore();
    }
}


void MainWindow::on_btn_refresh_com_clicked()
{
    refresh_cmd_port_list();
}


void MainWindow::on_pb_set_azimuth_clicked()
{
    RotatorProtocol rp;
    double azimuth = ui->le_azimuth->text().toDouble();
    rp.set_target_angle((int)(azimuth*100), 0, AZIMUTH);
    send_cmd_rotator(rp.get_bitstring());
}

