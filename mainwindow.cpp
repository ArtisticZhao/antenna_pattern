#include <cmath>
#include <QThread>
#include <QFileDialog>
#include <QSerialPortInfo>
#include <QStandardPaths>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    n9918a = new N9918a();
    polar_motor = new PolarMotor();
    rotator = new Rotator();
    measure_mission = nullptr;
    n9918a_status = "Disconnected";
    polar_motor_status = "Disconnected";
    rotator_status = "Disconnected";

    // process bar
    pProgressBar = new QProgressBar();
    pProgressBar->setRange(0, 100);
    ui->statusbar->addPermanentWidget(pProgressBar);

    // 创建信号连接
    connect(ui->cb_polar_motor_com, SIGNAL(combo_box_showpopup(QComboBoxMoreSignal*)), this, SLOT(on_cb_com_showPopup(QComboBoxMoreSignal*)));
    connect(ui->cb_rotator_com, SIGNAL(combo_box_showpopup(QComboBoxMoreSignal*)), this, SLOT(on_cb_com_showPopup(QComboBoxMoreSignal*)));
    
    connect(n9918a, SIGNAL(status_changed(DevStatus)), this, SLOT(on_n9918a_status_updated(DevStatus)));
    connect(n9918a, SIGNAL(measure_updated(double, double, double)), this, SLOT(on_n9918a_measure_updated(double, double, double)));
    connect(n9918a, SIGNAL(logging(LogLevel, QString)), this, SLOT(on_logging(LogLevel, QString)));

    connect(polar_motor, SIGNAL(status_changed()), this, SLOT(on_polar_motor_status_changed()));
    connect(polar_motor, SIGNAL(update_angle(double)), this, SLOT(on_polar_motor_angle_updated(double)));
    connect(polar_motor, SIGNAL(logging(LogLevel, QString)), this, SLOT(on_logging(LogLevel, QString)));

    connect(rotator, SIGNAL(status_changed()), this, SLOT(on_rotator_status_changed()));
    connect(rotator, SIGNAL(update_angle(double, double)), this, SLOT(on_rotator_angle_updated(double, double)));
    connect(rotator, SIGNAL(logging(LogLevel, QString)), this, SLOT(on_logging(LogLevel, QString)));

    
}

MainWindow::~MainWindow()
{
    delete n9918a;
    delete polar_motor;
    delete rotator;
    delete ui;
}

void MainWindow::statusBar_status(Module module, QString status) {
    switch (module) {
    case Module::N9918a:
        n9918a_status = status;
        break;
    case Module::Rotator:
        rotator_status = status;
        break;
    case Module::PolarMotor:
        polar_motor_status = status;
        break;
    default:
        break;
    }
    ui->statusbar->showMessage(QString("N9918A: %1 | Rotator: %2 | Polar Motor: %3").arg(n9918a_status).arg(rotator_status).arg(polar_motor_status));
}

QString MainWindow::select_save_file() {
	QString fileName;
    fileName = QFileDialog::getSaveFileName(this,
            tr("Save file"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), tr("CSV (*.csv)"));
    if (!fileName.isNull()) {
        //fileName是文件名
        return fileName;
    }else{
        //点的是取消
        return fileName;
    }
}

void MainWindow::on_logging(LogLevel level, QString msg) {
	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_date = current_date_time.toString("hh:mm:ss.zzz");

    switch (level) {
    case LogLevel::Info:
        ui->textEdit_log->append(QString("%1: [INFO] %2").arg(current_date).arg(msg));
        break;
    case LogLevel::Warnning:
        ui->textEdit_log->append(QString("<font color=\"#FF9912\">%1: [WARNNING] %2</font> ").arg(current_date).arg(msg));
        break;
    case LogLevel::Error:
        ui->textEdit_log->append(QString("<font color=\"#FF0000\">%1: [ERROR] %2</font> ").arg(current_date).arg(msg));
        break;
    default:
        break;
    }
}

void MainWindow::on_cb_com_showPopup(QComboBoxMoreSignal* combo_box) {
	combo_box->clear();
	// 列出可用串口
	foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts()) {
		combo_box->addItem(info.portName());
	}
}

void MainWindow::on_pb_n9918a_connect_clicked() {
    if (n9918a->deviceOK == DevStatus::disconnected) {
        n9918a->connectToN9918a(ui->le_n9918a_ip->text());
    }
    else {
        n9918a->disconnect();
    }
}

void MainWindow::on_n9918a_status_updated(DevStatus deviceOK) {
    switch (deviceOK) {
    case DevStatus::disconnected:
        ui->pb_n9918a_connect->setText(QStringLiteral("连接"));
        ui->le_n9918a_ip->setEnabled(true);
        ui->le_n9918a_center_freq->setEnabled(true);
        ui->le_n9918a_span_freq->setEnabled(true);
        ui->le_n9918a_sample->setEnabled(true);
        statusBar_status(Module::N9918a, "Disconnected");
        break;
    case DevStatus::connecting:
        ui->pb_n9918a_connect->setText(QStringLiteral("断开"));
		ui->le_n9918a_ip->setEnabled(false);
		ui->le_n9918a_center_freq->setEnabled(false);
		ui->le_n9918a_span_freq->setEnabled(false);
		ui->le_n9918a_sample->setEnabled(false);
        statusBar_status(Module::N9918a, "Connecting");
        break;
    case DevStatus::connected:
        ui->pb_n9918a_connect->setText(QStringLiteral("断开"));
		ui->le_n9918a_ip->setEnabled(false);
		ui->le_n9918a_center_freq->setEnabled(false);
		ui->le_n9918a_span_freq->setEnabled(false);
		ui->le_n9918a_sample->setEnabled(false);
        statusBar_status(Module::N9918a, "Connected");
        break;
    default:
        break;
    }
}

void MainWindow::on_n9918a_measure_updated(double max_power, double min_power, double max_freq) {
    ui->le_n9918a_maxpower->setText(QString::number(max_power));
    ui->le_n9918a_maxpower_freq->setText(QString::number(max_freq));
    ui->le_n9918a_minpower->setText(QString::number(min_power));
}

void MainWindow::on_pb_polar_motor_connect_clicked() {
    if (!polar_motor->comOk) {
        polar_motor->connectToCom(ui->cb_polar_motor_com->currentText());
    }
    else {
        polar_motor->disconnect();
    }
}

void MainWindow::on_pb_polar_motor_set_angle_clicked() {
    qDebug() << "on polar motor set angle";
    polar_motor->turn_to(ui->le_polar_motor_cmd_angle->text().toDouble());
}

void MainWindow::on_pb_polar_motor_reset_angle_clicked() {
    polar_motor->reset_angle();
}

void MainWindow::on_polar_motor_status_changed() {
    qDebug() << "on polar motor status changed" << polar_motor->comOk;
    if (polar_motor->comOk) {
        ui->pb_polar_motor_connect->setText(QStringLiteral("断开"));
        ui->cb_polar_motor_com->setEnabled(false);
        ui->pb_polar_motor_reset_angle->setEnabled(true);
        ui->pb_polar_motor_set_angle->setEnabled(true);
        statusBar_status(Module::PolarMotor, "Connected");
    }
    else {
        ui->pb_polar_motor_connect->setText(QStringLiteral("连接"));
        ui->cb_polar_motor_com->setEnabled(true);
		ui->pb_polar_motor_reset_angle->setEnabled(false);
		ui->pb_polar_motor_set_angle->setEnabled(false);
        statusBar_status(Module::PolarMotor, "Disconnected");
    }
}

void MainWindow::on_polar_motor_angle_updated(double angle) {
    ui->le_polar_motor_current_angle->setText(QString::number(angle));
}

void MainWindow::on_pb_rotator_connect_clicked() {
    if (!rotator->comOk) {
        rotator->connectToCom(ui->cb_rotator_com->currentText());
    }
    else {
        rotator->disconnect();
    }
}

void MainWindow::on_pb_rotator_set_pitch_clicked() {
    rotator->set_pitch(ui->le_rotator_cmd_pitch->text().toDouble());
}

void MainWindow::on_pb_rotator_set_azimuth_clicked() {
    rotator->set_azimuth(ui->le_rotator_cmd_azimuth->text().toDouble());
}

void MainWindow::on_rotator_status_changed() {
    qDebug() << "on rotator status changed" << rotator->comOk;
    if (rotator->comOk) {
        ui->pb_rotator_connect->setText(QStringLiteral("断开"));
		ui->cb_rotator_com->setEnabled(false);
		ui->pb_rotator_set_pitch->setEnabled(true);
		ui->pb_rotator_set_azimuth->setEnabled(true);
        statusBar_status(Module::Rotator, "Connected");
    } else {
		ui->pb_rotator_connect->setText(QStringLiteral("连接"));
		ui->cb_rotator_com->setEnabled(true);
		ui->pb_rotator_set_pitch->setEnabled(false);
		ui->pb_rotator_set_azimuth->setEnabled(false);
        statusBar_status(Module::Rotator, "Disconnected");
    }
}

void MainWindow::on_rotator_angle_updated(double pitch, double azimuth) {
    ui->le_rotator_current_pitch->setText(QString::number(pitch));
    ui->le_rotator_current_azimuth->setText(QString::number(azimuth));
}

void MainWindow::on_checkBox_polar_motor_preconfig_enable_stateChanged() {
    if (ui->checkBox_polar_motor_preconfig_enable->isChecked()) {
        ui->le_polar_motor_preconfig_start_angle->setEnabled(true);
        ui->le_polar_motor_preconfig_stop_angle->setEnabled(true);
        ui->le_polar_motor_preconfig_step_angle->setEnabled(true);
    }
    else {
		ui->le_polar_motor_preconfig_start_angle->setEnabled(false);
		ui->le_polar_motor_preconfig_stop_angle->setEnabled(false);
		ui->le_polar_motor_preconfig_step_angle->setEnabled(false);
    }
}

void MainWindow::on_checkBox_rotator_preconfig_enable_stateChanged() {
	if (ui->checkBox_rotator_preconfig_enable->isChecked()) {
		ui->le_rotator_preconfig_start_angle->setEnabled(true);
		ui->le_rotator_preconfig_stop_angle->setEnabled(true);
		ui->le_rotator_preconfig_step_angle->setEnabled(true);
	}
	else {
		ui->le_rotator_preconfig_start_angle->setEnabled(false);
		ui->le_rotator_preconfig_stop_angle->setEnabled(false);
		ui->le_rotator_preconfig_step_angle->setEnabled(false);
	}
}

void MainWindow::on_pb_start_mission_clicked() {
    if (ui->pb_start_mission->text() == QStringLiteral("停止任务")) {
        qDebug() << "stop mission";
        if (measure_mission != nullptr) {
            measure_mission->mission_stop();
        }
        ui->pb_start_mission->setText(QStringLiteral("开始任务"));
        return;
    }
    QString file_full = select_save_file();
	if (file_full.length() == 0) {
		// 用户取消选择
		return;
	}
    
    ui->pb_start_mission->setText(QStringLiteral("停止任务"));
    MissonType type;
    if (ui->rb_patten->isChecked()) {
        type = MissonType::RadiationPattern;
    }else if (ui->rb_polar->isChecked()) {
        type = MissonType::Polarization;
    }else if (ui->rb_patten3d->isChecked()) {
        type = MissonType::RadiationPattern3D;
    }


    if (measure_mission != nullptr) {
        delete measure_mission;
    }
    // Create mission
    measure_mission = new Mission(type,
        ui->checkBox_polar_motor_preconfig_enable->isChecked(),
        ui->le_polar_motor_preconfig_start_angle->text().toDouble(),
        ui->le_polar_motor_preconfig_stop_angle->text().toDouble(),
        ui->le_polar_motor_preconfig_step_angle->text().toDouble(),

        ui->checkBox_rotator_preconfig_enable->isChecked(),
		ui->le_rotator_preconfig_start_angle->text().toDouble(),
		ui->le_rotator_preconfig_stop_angle->text().toDouble(),
        ui->le_rotator_preconfig_step_angle->text().toDouble(),

        n9918a, polar_motor, rotator
        );
    
    measure_mission->set_pattern_widgets(ui->chart_spectrum, ui->chart_pattern);
    connect(measure_mission, SIGNAL(logging(LogLevel, QString)), this, SLOT(on_logging(LogLevel, QString)));
    connect(measure_mission, SIGNAL(status_changed(bool)), this, SLOT(on_mission_status_changed(bool)));
    connect(measure_mission, SIGNAL(process_changed(int)), this, SLOT(on_mission_process_changed(int)));

    n9918a->init(ui->le_n9918a_sample->text(), ui->le_n9918a_center_freq->text(), ui->le_n9918a_span_freq->text());

    measure_mission->mission_start(file_full);
    ui->pb_start_mission->setText(QStringLiteral("开始任务"));
}

void MainWindow::on_mission_status_changed(bool busy) {
	if (busy) {
		ui->le_n9918a_ip->setEnabled(false);
		ui->pb_n9918a_connect->setEnabled(false);
		ui->le_n9918a_center_freq->setEnabled(false);
		ui->le_n9918a_span_freq->setEnabled(false);
		ui->le_n9918a_sample->setEnabled(false);

		ui->cb_polar_motor_com->setEnabled(false);
		ui->pb_polar_motor_connect->setEnabled(false);
		ui->pb_polar_motor_set_angle->setEnabled(false);
		ui->pb_polar_motor_set_angle->setEnabled(false);
		ui->le_polar_motor_cmd_angle->setEnabled(false);

		ui->cb_rotator_com->setEnabled(false);
		ui->pb_rotator_connect->setEnabled(false);
		ui->le_rotator_cmd_azimuth->setEnabled(false);
		ui->le_rotator_cmd_pitch->setEnabled(false);
		ui->pb_rotator_set_azimuth->setEnabled(false);
		ui->pb_rotator_set_pitch->setEnabled(false);

		ui->rb_patten->setEnabled(false);
		ui->rb_polar->setEnabled(false);
		ui->checkBox_polar_motor_preconfig_enable->setEnabled(false);
		ui->le_polar_motor_preconfig_start_angle->setEnabled(false);
		ui->le_polar_motor_preconfig_stop_angle->setEnabled(false);
		ui->le_polar_motor_preconfig_step_angle->setEnabled(false);
		ui->checkBox_rotator_preconfig_enable->setEnabled(false);
		ui->le_rotator_preconfig_start_angle->setEnabled(false);
		ui->le_rotator_preconfig_stop_angle->setEnabled(false);
		ui->le_rotator_preconfig_step_angle->setEnabled(false);
	}
	else {
		ui->le_n9918a_ip->setEnabled(true);
		ui->pb_n9918a_connect->setEnabled(true);
		ui->le_n9918a_center_freq->setEnabled(true);
		ui->le_n9918a_span_freq->setEnabled(true);
		ui->le_n9918a_sample->setEnabled(true);

		ui->cb_polar_motor_com->setEnabled(true);
		ui->pb_polar_motor_connect->setEnabled(true);
		ui->pb_polar_motor_set_angle->setEnabled(true);
		ui->pb_polar_motor_set_angle->setEnabled(true);
		ui->le_polar_motor_cmd_angle->setEnabled(true);

		ui->cb_rotator_com->setEnabled(true);
		ui->pb_rotator_connect->setEnabled(true);
		ui->le_rotator_cmd_azimuth->setEnabled(true);
		ui->le_rotator_cmd_pitch->setEnabled(true);
		ui->pb_rotator_set_azimuth->setEnabled(true);
		ui->pb_rotator_set_pitch->setEnabled(true);

		ui->rb_patten->setEnabled(true);
		ui->rb_polar->setEnabled(true);
		ui->checkBox_polar_motor_preconfig_enable->setEnabled(true);
		ui->le_polar_motor_preconfig_start_angle->setEnabled(true);
		ui->le_polar_motor_preconfig_stop_angle->setEnabled(true);
		ui->le_polar_motor_preconfig_step_angle->setEnabled(true);
		ui->checkBox_rotator_preconfig_enable->setEnabled(true);
		ui->le_rotator_preconfig_start_angle->setEnabled(true);
		ui->le_rotator_preconfig_stop_angle->setEnabled(true);
		ui->le_rotator_preconfig_step_angle->setEnabled(true);
	}
}

void MainWindow::on_mission_process_changed(int process_val) {
    pProgressBar->setValue(process_val);
}

void MainWindow::on_rb_patten3d_toggled(bool checked) {
    static bool rotator_preconfig;
    static bool polar_preconfig;
    if (checked) {
        ui->label_rotator_pitch->setText(QStringLiteral("转台俯仰"));
        ui->label_polar_azimuth->setText(QStringLiteral("转台方位"));
        rotator_preconfig = ui->checkBox_rotator_preconfig_enable->isChecked();
        polar_preconfig = ui->checkBox_polar_motor_preconfig_enable->isChecked();
        ui->checkBox_rotator_preconfig_enable->setChecked(true);
        ui->checkBox_polar_motor_preconfig_enable->setChecked(true);
		ui->checkBox_rotator_preconfig_enable->setEnabled(false);
		ui->checkBox_polar_motor_preconfig_enable->setEnabled(false);
    }
    else {
		ui->label_rotator_pitch->setText(QStringLiteral("转台"));
        ui->label_polar_azimuth->setText(QStringLiteral("极化电机"));
		ui->checkBox_rotator_preconfig_enable->setChecked(rotator_preconfig);
		ui->checkBox_polar_motor_preconfig_enable->setChecked(polar_preconfig);
		ui->checkBox_rotator_preconfig_enable->setEnabled(true);
		ui->checkBox_polar_motor_preconfig_enable->setEnabled(true);
    }
}
