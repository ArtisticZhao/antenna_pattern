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
	bool comOk;         //�����Ƿ��
	double current_angle;

	PolarMotor();
	~PolarMotor();
	void polar_motor_connect(QString com);
	void polar_motor_disconnect();
	bool polar_motor_turn(double angle);
	void polar_motor_reset();


private:
	QextSerialPort* motor_com;       // ����ͨ�Ŷ���
	QTimer* timerReadMotor;          // ��ʱ��ȡ��������
	bool motor_cmd_lock;             // �������ȴ�

	void polar_motor_send_cmd(const QByteArray& cmd);
	bool polar_motor_wait_turn(double angle);
	bool polar_motor_wait_reset();


private slots:
	void on_polar_motor_read_data();
};

