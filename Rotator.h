#pragma once
#include <QTimer>
#include <QDataStream>
#include "qextserialport.h"
#include "logging.h"
#include "MotorCtrl.h"

class Rotator : public QObject, public MotorCtrl{
	Q_OBJECT

signals:
	void status_changed();
	void update_angle(double pitch, double azimuth);
	void logging(LogLevel level, QString msg);


public:
	bool comOk = false;         //串口是否打开
	double current_pitch;
	double current_azimuth;

	Rotator();
	~Rotator();
	void connectToCom(QString com_port);
	void disconnect();
	bool turn_to(double azimuth);
	bool turn_pitch_to(double angle);
	void turn_to_zero();
	void set_pitch(double pitch);
	void set_azimuth(double azimuth);


private:
	QextSerialPort* com;        //串口通信对象
	QTimer* timerRead;          //定时读取串口数据

	bool send_cmd(const QByteArray& cmd);
	bool is_over_leap;       // 在转台接近360°的时候, 可能因为pid的原因显示0.01°左右
	                         // 这个标志用来区分当前是否是转过一圈的了.


private slots :
	void read_data();
};

