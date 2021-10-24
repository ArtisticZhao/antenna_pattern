#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QCloseEvent>
#include <QtCharts>

#include "qextserialport.h"

#include "N9918a.h"
#include "PolarMotor.h"
#include "Rotator.h"
#include "QComboBoxMoreSignal.h"
#include "logging.h"

QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum class Module {
    N9918a,
    Rotator,
    PolarMotor
};

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
    // modules
    N9918a* n9918a;
    PolarMotor* polar_motor;
    Rotator* rotator;
    QString n9918a_status;
    QString rotator_status;
    QString polar_motor_status;
    // 绘图相关
    QChart *c;
    QPolarChart *chart;
    QSplineSeries *pattern_data;


    void statusBar_status(Module module, QString status);


// 事件响应
private slots:
    void on_logging(LOGLEVEL level, QString msg);
    void on_cb_com_showPopup(QComboBoxMoreSignal* combo_box);

    void on_pb_n9918a_connect_clicked();
    void on_n9918a_status_updated(DEV_STATUS deviceOK);
    void on_n9918a_measure_updated(double max_power, double min_power, int max_index);

    void on_pb_polar_motor_connect_clicked();
    void on_pb_polar_motor_set_angle_clicked();
    void on_pb_polar_motor_reset_angle_clicked();
    void on_polar_motor_status_changed();
    void on_polar_motor_angle_updated(double angle);

    void on_pb_rotator_connect_clicked();
    void on_pb_rotator_set_pitch_clicked();
    void on_pb_rotator_set_azimuth_clicked();
    void on_rotator_status_changed();
    void on_rotator_angle_updated(double pitch, double azimuth);
};
#endif // MAINWINDOW_H
