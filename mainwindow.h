#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QCloseEvent>
//#include <qwt/qwt_plot_curve.h>
//#include <qwt/qwt_plot_grid.h>
//#include <qwtpolar/qwt_polar_plot.h>
#include "QTelnet.h"
#include "qextserialport.h"
//#include "plot.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    //这是一个虚函数，继承自QEvent.只要重写了这个虚函数，当你按下窗口右上角的"×"时，就会调用你所重写的此函数.
    void closeEvent(QCloseEvent*event);

private:
    void set_status_text(const QString &n9918a, const QString &rotator);  // 设置底部状态栏

    // 9918相关函数
    void init_n9918a();
    void send_cmd_9918a(const QString &cmd);
    void measure_power();
    void freq_linespace();
//    void draw_spectrum(QVector<double>* xaxis, QVector<double>* spectrum_data);
//    void draw_pattern();

    // 转台相关函数
    void refresh_cmd_port_list();
    void send_cmd_rotator(const QByteArray &cmd);
    void turn_rotator(double azimuth);

    // file
    QString select_save_file();

    // 防呆
    void set_n9918_config_enable(bool enable);
    void set_rotator_config_enable(bool enable);
    void on_process_enable(bool enable);    // 当测试程序开始 结束时 改变一些界面的状态



private slots:
    void on_lan_connect_clicked();
    void on_state_changed(QAbstractSocket::SocketState s);
    void com_read_data();
    void on_start_test_clicked();
    void on_n9918_log_textChanged();
    void on_com_connect_clicked();
    void on_set_pitch_clicked();
    void on_rotator_log_textChanged();
    void on_stop_test_clicked();

    void on_btn_refresh_com_clicked();

    void on_pb_set_azimuth_clicked();

public slots:
    void add_n9918a_log(const char *msg, int count);

private:
    Ui::MainWindow *ui;
//    QwtPolarPlot plot;
    QTelnet telnet;
    QString status_n9918a, status_rotator;
    bool cmd_lock;  // 命令锁，如果命令是带有？查询语句，需要等待结果返回
    QString last_9918_anser;
    QVector<double>* xaxis;    // 保存频谱数据的
//    Plot *d_plot;
//    QwtSeriesData<QwtPointPolar> *pattern;

    bool comOk = false;                 //串口是否打开
    QextSerialPort *com;        //串口通信对象
    QTimer *timerRead;          //定时读取串口数据
    bool kill_process = false;

    bool on_process =false;             // 测试中

};
#endif // MAINWINDOW_H
