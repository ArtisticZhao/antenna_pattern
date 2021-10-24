#include "PolarMotor.h"
#include <QDebug>
   
PolarMotor::PolarMotor() {
	motor_comOk = false;
	motor_com = nullptr;
	timerReadMotor = new QTimer(this);
	timerReadMotor->setInterval(100);
	connect(timerReadMotor, SIGNAL(timeout()), this, SLOT(on_motor_com_read_data()));
}

void PolarMotor::polar_motor_connect(QString com) {
	if (!motor_comOk) {
		// 初始化串口
		motor_com = new QextSerialPort(com, QextSerialPort::Polling);
		motor_comOk = motor_com->open(QIODevice::ReadWrite);
		if (motor_comOk) {
			//清空缓冲区
			motor_com->flush();
			//设置波特率
			motor_com->setBaudRate(BAUD9600);
			//设置数据位
			motor_com->setDataBits(DATA_8);
			//设置校验位
			motor_com->setParity(PAR_NONE);
			//设置停止位
			motor_com->setStopBits(STOP_1);
			motor_com->setFlowControl(FLOW_OFF);
			motor_com->setTimeout(10);
			//读取数据
			timerReadMotor->start();
			// call back ok
		}
	}
}

void PolarMotor::polar_motor_turn(double angle) {
	double d_angle = angle * 100;
	if (d_angle < 0) {
		polar_motor_send_cmd(QString("cmd %1 0").arg((int)(-d_angle)).toUtf8());
	}
	else {
		polar_motor_send_cmd(QString("cmd %1 1").arg((int)(d_angle)).toUtf8());
	}
}

void PolarMotor::polar_motor_disconnect() {
	if (motor_comOk)
	{
		// 断开串口
		timerReadMotor->stop();
		motor_comOk = false;
		motor_com->close();
		motor_com->deleteLater();
	}
}

void PolarMotor::polar_motor_send_cmd(const QByteArray& cmd) {
	if (motor_comOk) {
		motorOk = false;
		motor_com->write(cmd);
	}
	int timeout = 100;
	// waiting for reply
	while (!motorOk) {
		QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
		timeout--;
		if (timeout == 0) {
			qDebug() << "motor timeout!";
			break;
		}
		QThread::msleep(50);
	}
}

void PolarMotor::on_polar_motor_read_data() {
	if (motor_com->bytesAvailable() <= 0) {
		return;
	}
	QByteArray data = motor_com->readAll();
	int dataLen = data.length();
	if (dataLen <= 0) {
		return;
	}
	int header = data.indexOf('O');
	if (header >= 0) {
		motorOk = true;
		qDebug() << "motor ok";
	}
	qDebug() << data;
}

