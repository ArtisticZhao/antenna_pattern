#include <QThread>
#include <QCoreApplication>
#include "Rotator.h"
#include "rotatorprotocol.h"

Rotator::Rotator() {
	// 初始化读串口定时器
	timerRead = new QTimer(this);
	timerRead->setInterval(100);
	connect(timerRead, SIGNAL(timeout()), this, SLOT(read_data()));
}

Rotator::~Rotator() {
	disconnect();
	delete timerRead;
}

void Rotator::connectToCom(QString com_port) {
	if (!comOk) {
		// 初始化串口
		com = new QextSerialPort(com_port, QextSerialPort::Polling);
		comOk = com->open(QIODevice::ReadWrite);
		if (comOk) {
			//清空缓冲区
			com->flush();
			//设置波特率
			com->setBaudRate(BAUD115200);
			//设置数据位
			com->setDataBits(DATA_8);
			//设置校验位
			com->setParity(PAR_NONE);
			//设置停止位
			com->setStopBits(STOP_1);
			com->setFlowControl(FLOW_OFF);
			com->setTimeout(10);

			//读取数据
			timerRead->start();
			emit status_changed();
		}
	}
}

void Rotator::disconnect() {
	if (comOk) {
		// 断开串口
		timerRead->stop();
		comOk = false;
		com->close();
		com->deleteLater();
		emit status_changed();
	}
}

bool Rotator::turn_to(double azimuth) {
	RotatorProtocol rp;
	rp.set_target_angle((int)(azimuth * 100), 0, RotatorProtocol::AZIMUTH);
	bool res = send_cmd(rp.get_bitstring());
	if (!res) return false;
	// wait rotator position
	int timeout = 100;
	double delta = azimuth - current_azimuth;

	while (!(-0.1 < delta && delta < 0.1)) {
		QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
		delta = azimuth - current_azimuth;
		timeout--;
		if (timeout == 0) {
			emit logging(LogLevel::Warnning, "Rotator timeout!");
			return false;
		}
		QThread::msleep(50);
	}
	//if (azimuth == 360 && (current_azimuth > 0 || current_azimuth)) {
	if (azimuth == 360 && (current_azimuth > 0 )) {
		is_over_leap = true;
		emit logging(LogLevel::Warnning, QString("turn 360 over leap!\n"));
	}
	emit logging(LogLevel::Info, QString("turn to the position! %1\n").arg(current_azimuth));
	return true;
}

void Rotator::turn_to_zero() {
	emit logging(LogLevel::Warnning, QString("Rotator turn to ZERO\n"));
	emit logging(LogLevel::Warnning, QString("current_azimuth %1, is overleap %2\n").arg(current_azimuth).arg(is_over_leap));
	if (current_azimuth > 0 && current_azimuth < 1 && is_over_leap) {
		set_azimuth(240);
		set_azimuth(120);
		set_azimuth(0);
	}
	else if (current_azimuth > 359 && is_over_leap) {
		set_azimuth(240);
		set_azimuth(120);
		set_azimuth(0);
	}
	else if (current_azimuth > 240 && current_azimuth < 359) {  // 需要避过0附近 359.xx 的角度!
		set_azimuth(240);
		set_azimuth(120);
		set_azimuth(0);
	}
	else if (current_azimuth > 120) {
		set_azimuth(120);
	}
	else {
		set_azimuth(0);
	}
	is_over_leap = false;
}

void Rotator::set_pitch(double pitch) {
	RotatorProtocol rp;
	rp.set_target_angle(0, (int)(pitch * 100), RotatorProtocol::PITCH);
	send_cmd(rp.get_bitstring());
}

void Rotator::set_azimuth(double azimuth) {
	RotatorProtocol rp;
	rp.set_target_angle((int)(azimuth * 100), 0, RotatorProtocol::AZIMUTH);
	send_cmd(rp.get_bitstring());
}

bool Rotator::send_cmd(const QByteArray& cmd) {
	if (comOk) {
		com->write(cmd);
		return true;
	}
	else {
		emit logging(LogLevel::Error, "Rotator not ready!!!");
		return false;
	}
}

void Rotator::read_data() {
	if (com->bytesAvailable() <= 0) {
		return;
	}
	QByteArray data = com->readAll();
	int dataLen = data.length();
	if (dataLen <= 0) {
		return;
	}
	int header = data.indexOf(0xaa);
	if (header == 0) {
		// 发现帧头, 解析角度
		// pitch
		char pitch[2] = { data[8], data[7] };
		QByteArray pitch_a = QByteArray(pitch, 2);
		QDataStream dataStream(pitch_a);
		qint16 pitch_int;
		dataStream >> pitch_int;
		double pitch_d = ((double)pitch_int) / 100;
		// azimuth
		char azimuth[2] = { data[6], data[5] };
		QByteArray azimuth_a = QByteArray(azimuth, 2);

		QDataStream dataStream_azimuth(azimuth_a);
		quint16 azimuth_int;
		dataStream_azimuth >> azimuth_int;
		double azimuth_d = ((double)azimuth_int) / 100;
		if ((azimuth_d != current_azimuth) || (pitch_d != current_pitch)) {
			current_pitch = pitch_d;
			current_azimuth = azimuth_d;
			emit update_angle(current_pitch, current_azimuth);
		}
	}
}