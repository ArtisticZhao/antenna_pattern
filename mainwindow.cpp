#include<QThread>
#include <QMessageBox>
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
}

MainWindow::~MainWindow()
{
    telnet.disconnectFromHost();
    delete ui;
}

void MainWindow::on_state_changed(QAbstractSocket::SocketState s)
{
    switch( s )
        {
        case QAbstractSocket::UnconnectedState:
            set_status_text("disconnected", "");
            ui->lan_connect->setText("1-连接");
            break;
        case QAbstractSocket::HostLookupState:
            set_status_text("connecting", "");
            ui->lan_connect->setText("断开");
            break;
        case QAbstractSocket::ConnectingState:
            set_status_text("connecting", "");
            ui->lan_connect->setText("断开");
            break;
        case QAbstractSocket::ConnectedState:
            set_status_text("connected", "");
            ui->lan_connect->setText("断开");
            break;
        case QAbstractSocket::BoundState:
            set_status_text("connected", "");
            ui->lan_connect->setText("断开");
            break;
        case QAbstractSocket::ListeningState:
            set_status_text("connected", "");
            ui->lan_connect->setText("断开");
            break;
        case QAbstractSocket::ClosingState:
            set_status_text("disconnected", "");
            ui->lan_connect->setText("断开");
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
        char pitch[2] = {data[8], data[7]};
        QByteArray pitch_a = QByteArray(pitch, 2);
        QDataStream dataStream(pitch_a);
        qint16 pitch_int;
        dataStream >> pitch_int;
        ui->current_pitch->setText(QString::number(((double)pitch_int)/100));
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
}


void MainWindow::on_lan_connect_clicked()
{
    if (! telnet.isConnected()){
        telnet.connectToHost(ui->ip_addr->text(), 5025);
        ui->lan_connect->setText("断开");
    }else{
        telnet.disconnectFromHost();
        ui->lan_connect->setText("1-连接");
    }

}

void MainWindow::add_n9918a_log(const char *msg, int count)
{
    ui->n9918_log->insertPlainText(QByteArray(msg, count));
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
        qDebug()<< "waiting for recv";
        int timeout = 100;
        while(cmd_lock){
            QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
            timeout --;
            if (timeout == 0){
                break;
                ui->n9918_log->insertPlainText("Timeout!\n");
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
}

void MainWindow::send_cmd_rotator(const QByteArray &cmd)
{
    if (comOk){
        com->write(cmd);
    }
}

void MainWindow::on_start_test_clicked()
{
    init_n9918a();
    //test
    measure_power();
}


void MainWindow::on_n9918_log_textChanged()
{
    ui->n9918_log->moveCursor(QTextCursor::End);
}


void MainWindow::on_com_connect_clicked()
{
    if (!comOk){
        // 初始化串口
        com = new QextSerialPort(ui->com_port->text(), QextSerialPort::Polling);
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
            ui->com_connect->setText("断开");
            set_status_text("", "connected");
            //读取数据
            timerRead->start();
        }
    }else{
        // 断开串口
        timerRead->stop();
        com->close();
        com->deleteLater();
        ui->com_connect->setText("2-连接");
        set_status_text("", "disconnected");
        comOk = false;
    }

}




void MainWindow::on_set_pitch_clicked()
{
    RotatorProtocol rp;
    double pitch = ui->pitch->text().toDouble();
    rp.set_target_angle(0, (int)(pitch*100), PITCH);
    qDebug()<< rp.get_bitstring();
    send_cmd_rotator(rp.get_bitstring());
}

