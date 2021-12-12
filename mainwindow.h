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
#include "Mission.h"
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


private:
    Ui::MainWindow *ui;
    QProgressBar* pProgressBar;
    // modules
    N9918a* n9918a;
    PolarMotor* polar_motor;
    Rotator* rotator;
    QString n9918a_status;
    QString rotator_status;
    QString polar_motor_status;
    Mission* measure_mission;
   
    void statusBar_status(Module module, QString status);
    QString select_save_file();


// 事件响应
private slots:
    void on_logging(LogLevel level, QString msg);
    void on_cb_com_showPopup(QComboBoxMoreSignal* combo_box);
    // n9918a
    void on_pb_n9918a_connect_clicked();
    void on_n9918a_status_updated(DevStatus deviceOK);
    void on_n9918a_measure_updated(double max_power, double min_power, double max_freq);
    // polar motor
    void on_pb_polar_motor_connect_clicked();
    void on_pb_polar_motor_set_angle_clicked();
    void on_pb_polar_motor_reset_angle_clicked();
    void on_polar_motor_status_changed();
    void on_polar_motor_angle_updated(double angle);
    // rotator
    void on_pb_rotator_connect_clicked();
    void on_pb_rotator_set_pitch_clicked();
    void on_pb_rotator_set_azimuth_clicked();
    void on_rotator_status_changed();
    void on_rotator_angle_updated(double pitch, double azimuth);
    // ui
    void on_checkBox_polar_motor_preconfig_enable_stateChanged();
    void on_checkBox_rotator_preconfig_enable_stateChanged();
    // mission
    void on_pb_start_mission_clicked();
    void on_mission_status_changed(bool busy);
    void on_mission_process_changed(int process_val);
    void on_rb_patten3d_toggled(bool checked);
};
#endif // MAINWINDOW_H
