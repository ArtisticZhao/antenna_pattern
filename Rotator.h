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
	void turn_to_zero();
	void set_pitch(double pitch);
	void set_azimuth(double azimuth);


private:
	QextSerialPort* com;        //串口通信对象
	QTimer* timerRead;          //定时读取串口数据

	bool send_cmd(const QByteArray& cmd);


private slots :
	void read_data();
};

