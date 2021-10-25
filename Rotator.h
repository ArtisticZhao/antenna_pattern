#pragma once
#include <QTimer>
#include <QDataStream>
#include "qextserialport.h"
#include "logging.h"

class Rotator : public QObject{
	Q_OBJECT

signals:
	void status_changed();
	void update_angle(double pitch, double azimuth);
	void logging(LOGLEVEL level, QString msg);


public:
	bool comOk = false;         //�����Ƿ��
	double current_pitch;
	double current_azimuth;

	Rotator();
	~Rotator();
	void connectToCom(QString com_port);
	void disconnect();
	void turn_to(double azimuth);
	void turn_to_zero();
	void set_pitch(double pitch);
	void set_azimuth(double azimuth);


private:
	bool kill_process = false;  //ֹͣ����
	QextSerialPort* com;        //����ͨ�Ŷ���
	QTimer* timerRead;          //��ʱ��ȡ��������

	bool send_cmd(const QByteArray& cmd);


private slots :
	void read_data();
};

