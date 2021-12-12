#include "PolarMotor.h"
#include <QDebug>
   
PolarMotor::PolarMotor() {
	comOk = false;
	current_angle = -999;
	motor_com = nullptr;
	motor_cmd_lock = false;
	
	timerReadMotor = new QTimer(this);
	timerReadMotor->setInterval(500);
	connect(timerReadMotor, SIGNAL(timeout()), this, SLOT(read_data()));
}

PolarMotor::~PolarMotor() {
	disconnect();
	timerReadMotor->deleteLater();
}

void PolarMotor::connectToCom(QString com) {
	if (!comOk) {
		// 初始化串口
		motor_com = new QextSerialPort(com, QextSerialPort::Polling);
		comOk = motor_com->open(QIODevice::ReadWrite);
		if (comOk) {
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
			emit status_changed();
		}
	}
}

void PolarMotor::disconnect() {
	if (comOk) {
		// 断开串口
		timerReadMotor->stop();
		comOk = false;
		motor_cmd_lock = false;
		motor_com->close();
		motor_com->deleteLater();

		emit status_changed();
	}
}

bool PolarMotor::turn_to(double angle) {
	emit logging(LogLevel::Info, QString("Polar motor set to %1").arg(angle));
	if (motor_cmd_lock) {
		emit logging(LogLevel::Warnning, QString("polar motor waiting lock"));
		qDebug() << "motor_cmd_lock";
		return false;
	}
	if (!(angle>=0 && angle <= 360)) {
		emit logging(LogLevel::Error, QString("input must in range 0 to 360: %1").arg(angle));
		qDebug() << "error range of polar motor turn" << angle;
		return false;
	}
	double d_angle = angle * 100;
	
	bool res = send_cmd(QString("cmd %1\n").arg((int)(d_angle)).toUtf8());
	if (!res) return false;  // send cmd failed, turn.
	for (int i = 0; i < 100; i++) {
		bool res = wait_turn(angle);
		if (res) {
			return true;
		}
		emit logging(LogLevel::Warnning, QString("Resend polar motor cmd!"));
		send_cmd(QString("cmd %1\n").arg((int)(d_angle)).toUtf8());
	}
	emit logging(LogLevel::Error, QString("polar motor timeout 100 times!!!"));
	return false;
}

bool PolarMotor::turn_pitch_to(double angle) {
	emit logging(LogLevel::Error, QString("polar motor has no implementation for turn_pitch_to!"));
	return false;
}

void PolarMotor::reset_angle() {
	send_cmd(QString("rst").toUtf8());
	wait_reset();
}

void PolarMotor::turn_to_zero() {
	turn_to(0);
}

bool PolarMotor::send_cmd(const QByteArray& cmd) {
	if (comOk) {
		motor_cmd_lock = true;
		motor_com->write(cmd);
		return true;
	} else {
		emit logging(LogLevel::Error, "Polar motor not ready!!!");
		return false;
	}
}

bool PolarMotor::wait_turn(double angle) {
	int timeout = 100;
	// waiting for reply
	while (motor_cmd_lock) {
		if ((current_angle - angle) < 2 && (current_angle - angle) > -2) {
			motor_cmd_lock = false;
			emit logging(LogLevel::Info, QString("polar motor turn to %1").arg(angle));
			qDebug() << "motor turn ok";
			return true;
		}
		QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
		timeout--;
		if (timeout == 0) {
			emit logging(LogLevel::Warnning, QString("polar motor turn timeout"));
			qDebug() << "motor timeout!";
			motor_cmd_lock = false;
			return false;
		}
		QThread::msleep(50);
	}
}

bool PolarMotor::wait_reset() {
	int timeout = 100;
	double angle = 0;
	// waiting for reply
	while (motor_cmd_lock) {
		if (current_angle == angle) {
			motor_cmd_lock = false;
			emit logging(LogLevel::Warnning, QString("polar motor reset OK"));
			qDebug() << "motor turn ok";
			return true;
		}
		QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
		timeout--;
		if (timeout == 0) {
			emit logging(LogLevel::Warnning, QString("polar motor reset timeout"));
			qDebug() << "motor timeout!";
			motor_cmd_lock = false;
			return false;
		}
		QThread::msleep(50);
	}
}

void PolarMotor::read_data() {
	if (motor_com->bytesAvailable() <= 0) {
		return;
	}
	QByteArray data = motor_com->readAll();
	int dataLen = data.length();
	if (dataLen <= 0) {
		return;
	}
	qDebug() << data;
	QString str = data;
	QStringList  strs = str.split(" ");
	if (strs.length() == 2) {
		strs[1].chop(1);  // delete last 1 char.
		double angle = strs[1].toDouble() / 100;
		if (angle != current_angle) {
			qDebug() << "update polar angle";
			current_angle = angle;
			emit update_angle(current_angle);
		}
	}
}

