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
	bool motor_comOk;         //�����Ƿ��
	QextSerialPort* motor_com;        //����ͨ�Ŷ���
	QTimer* timerReadMotor;          //��ʱ��ȡ��������
	bool motorOk;

	void polar_motor_send_cmd(const QByteArray& cmd);
private slots:
	void on_polar_motor_read_data();
};

