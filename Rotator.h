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
	bool comOk = false;         //�����Ƿ��
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
	QextSerialPort* com;        //����ͨ�Ŷ���
	QTimer* timerRead;          //��ʱ��ȡ��������

	bool send_cmd(const QByteArray& cmd);
	bool is_over_leap;       // ��ת̨�ӽ�360���ʱ��, ������Ϊpid��ԭ����ʾ0.01������
	                         // �����־�������ֵ�ǰ�Ƿ���ת��һȦ����.


private slots :
	void read_data();
};

