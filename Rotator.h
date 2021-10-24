#pragma once
#include <QTimer>
#include <QDataStream>
#include "qextserialport.h"

class Rotator : public QObject{
private:
	bool comOk = false;         //串口是否打开
	bool kill_process = false;  //停止进程
	QextSerialPort* com;        //串口通信对象
	QTimer* timerRead;          //定时读取串口数据

	void rotator_send_cmd(const QByteArray& cmd);
	double current_pitch, current_azimuth;

public:
	Rotator();
	void rotator_connect(QString com_port);
	void rotator_disconnect();
	
	void rotator_read_data();
	void rotator_turn(double azimuth);
	void rotator_set_pitch(double pitch);
	void rotator_set_azimuth(double azimuth);

	typedef void (*callback)(QString msg);


};

