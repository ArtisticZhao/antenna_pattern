#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QCloseEvent>
#include <QtCharts>


#include "qextserialport.h"

QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
//
//protected:
//    //这是一个虚函数，继承自QEvent.只要重写了这个虚函数，当你按下窗口右上角的"×"时，就会调用你所重写的此函数.
//    /*void closeEvent(QCloseEvent*event);*/
//
//private:
//    void set_status_text(const QString &n9918a, const QString &rotator);  // 设置底部状态栏
//
//    // 9918相关函数
////    void draw_spectrum(QVector<double>* xaxis, QVector<double>* spectrum_data);
//    void draw_pattern(double max);
//
//    // 转台相关函数
//    void refresh_cmd_port_list();
//
//
//    // motor
//    
//
//
//    // file
//    QString select_save_file();
//
//    // 防呆
//    void set_n9918_config_enable(bool enable);
//    void set_rotator_config_enable(bool enable);
//    void on_process_enable(bool enable);    // 当测试程序开始 结束时 改变一些界面的状态
//
//
//
//private slots:
//
//    // rotator
//    void on_com_connect_clicked();
//    void on_btn_refresh_com_clicked();
//    void on_rotator_log_textChanged();
//
//    void on_set_pitch_clicked();
//    void on_pb_set_azimuth_clicked();
//
//
//    // motor
//    void on_com_connect_motor_clicked();
//    void on_btn_refresh_com_motor_clicked();
//    void on_pb_turn_motor_clicked();
//
//
//    // test
//    void on_start_test_clicked();
//    void on_stop_test_clicked();
//

private:
    Ui::MainWindow *ui;
    
    QString status_rotator;
    
    

    // 绘图相关
    QChart *c;
    QPolarChart *chart;
    QSplineSeries *pattern_data;

    // rotator
   
    

    // motor

    // test
    bool kill_process = false;
    bool on_process =false;             // 测试中

};
#endif // MAINWINDOW_H
