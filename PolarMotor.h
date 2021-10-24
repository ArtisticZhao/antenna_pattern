#pragma once
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include "qextserialport.h"
#include "logging.h"


class PolarMotor : public QObject{
	Q_OBJECT

signals:
	void polor_motor_status_changed();
	void polor_motor_update_angle(double angle);
	void polar_motor_logging(LOGLEVEL level, QString msg);


public:
	bool comOk;         //串口是否打开
	double current_angle;

	PolarMotor();
	~PolarMotor();
	void polar_motor_connect(QString com);
	void polar_motor_disconnect();
	bool polar_motor_turn(double angle);
	void polar_motor_reset();


private:
	QextSerialPort* motor_com;       // 串口通信对象
	QTimer* timerReadMotor;          // 定时读取串口数据
	bool motor_cmd_lock;             // 命令结果等待

	void polar_motor_send_cmd(const QByteArray& cmd);
	bool polar_motor_wait_turn(double angle);
	bool polar_motor_wait_reset();


private slots:
	void on_polar_motor_read_data();
};

