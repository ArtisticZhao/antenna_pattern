#pragma once
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include "qextserialport.h"
#include "logging.h"


class PolarMotor : public QObject{
	Q_OBJECT

signals:
	void status_changed();
	void update_angle(double angle);
	void logging(LOGLEVEL level, QString msg);


public:
	bool comOk;         //串口是否打开
	double current_angle;

	PolarMotor();
	~PolarMotor();
	void connectToCom(QString com);
	void disconnect();
	bool turn_to(double angle);
	void reset_angle();
	void turn_to_zero();


private:
	QextSerialPort* motor_com;       // 串口通信对象
	QTimer* timerReadMotor;          // 定时读取串口数据
	bool motor_cmd_lock;             // 命令结果等待

	bool send_cmd(const QByteArray& cmd);
	bool wait_turn(double angle);
	bool wait_reset();


private slots:
	void read_data();
};

