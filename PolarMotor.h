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
	bool comOk;         //�����Ƿ��
	double current_angle;

	PolarMotor();
	~PolarMotor();
	void connectToCom(QString com);
	void disconnect();
	bool turn_to(double angle);
	void reset_angle();
	void turn_to_zero();


private:
	QextSerialPort* motor_com;       // ����ͨ�Ŷ���
	QTimer* timerReadMotor;          // ��ʱ��ȡ��������
	bool motor_cmd_lock;             // �������ȴ�

	bool send_cmd(const QByteArray& cmd);
	bool wait_turn(double angle);
	bool wait_reset();


private slots:
	void read_data();
};

