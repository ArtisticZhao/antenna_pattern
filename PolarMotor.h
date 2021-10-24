#pragma once
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include "qextserialport.h"
class PolarMotor : public QObject{
public:
	PolarMotor();
	void polar_motor_connect(QString com);
	void polar_motor_turn(double angle);
	void polar_motor_disconnect();
private:
	bool motor_comOk;         //串口是否打开
	QextSerialPort* motor_com;        //串口通信对象
	QTimer* timerReadMotor;          //定时读取串口数据
	bool motorOk;

	void polar_motor_send_cmd(const QByteArray& cmd);
private slots:
	void on_polar_motor_read_data();
};

