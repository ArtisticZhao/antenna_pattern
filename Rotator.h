#pragma once
#include <QTimer>
#include <QDataStream>
#include "qextserialport.h"

class Rotator : public QObject{
private:
	bool comOk = false;         //�����Ƿ��
	bool kill_process = false;  //ֹͣ����
	QextSerialPort* com;        //����ͨ�Ŷ���
	QTimer* timerRead;          //��ʱ��ȡ��������

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

