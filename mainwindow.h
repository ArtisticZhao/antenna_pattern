#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "QTelnet.h"
#include "qextserialport.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void set_status_text(const QString &n9918a, const QString &rotator);  // 设置底部状态栏

    // 9918相关函数
    void init_n9918a();
    void send_cmd_9918a(const QString &cmd);
    void measure_power();

    // 转台相关函数
    void send_cmd_rotator(const QByteArray &cmd);


private slots:
    void on_lan_connect_clicked();
    void on_state_changed(QAbstractSocket::SocketState s);
    void com_read_data();

    void on_start_test_clicked();

    void on_n9918_log_textChanged();

    void on_com_connect_clicked();

    void on_set_pitch_clicked();

public slots:
    void add_n9918a_log(const char *msg, int count);

private:
    Ui::MainWindow *ui;
    QTelnet telnet;
    QString status_n9918a, status_rotator;
    bool cmd_lock;  // 命令锁，如果命令是带有？查询语句，需要等待结果返回

    bool comOk = false;                 //串口是否打开
    QextSerialPort *com;        //串口通信对象
    QTimer *timerRead;          //定时读取串口数据

};
#endif // MAINWINDOW_H
