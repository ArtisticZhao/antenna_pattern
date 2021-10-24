#include <QThread>
#include <QCoreApplication>
#include "Rotator.h"
#include "rotatorprotocol.h"

Rotator::Rotator() {
	// 初始化读串口定时器
	timerRead = new QTimer(this);
	timerRead->setInterval(100);
	connect(timerRead, SIGNAL(timeout()), this, SLOT(rotator_read_data()));
}

void Rotator::rotator_connect(QString com_port) {
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
		}
	}
}

void Rotator::rotator_disconnect() {
	if (comOk) {
		// 断开串口
		timerRead->stop();
		comOk = false;
		com->close();
		com->deleteLater();
	}
}

void Rotator::rotator_send_cmd(const QByteArray& cmd) {
	if (comOk) {
		com->write(cmd);
	}
}

void Rotator::rotator_read_data() {
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
		current_pitch = ((double)pitch_int) / 100;
		// azimuth
		char azimuth[2] = { data[6], data[5] };
		QByteArray azimuth_a = QByteArray(azimuth, 2);

		QDataStream dataStream_azimuth(azimuth_a);
		quint16 azimuth_int;
		dataStream_azimuth >> azimuth_int;
		current_azimuth = ((double)azimuth_int) / 100;
	}
}

void Rotator::rotator_turn(double azimuth) {
	RotatorProtocol rp;
	rp.set_target_angle((int)(azimuth * 100), 0, AZIMUTH);
	rotator_send_cmd(rp.get_bitstring());
	// wait rotator position
	int timeout = 100;
	double delta = azimuth - current_azimuth;

	while (!(-0.1 < delta && delta < 0.1)) {
		QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
		delta = azimuth - current_azimuth;
		if (kill_process) {
			return;
		}
		timeout--;
		if (timeout == 0) {
			/*ui->rotator_log->insertPlainText("Timeout!\n");*/
			break;
		}
		QThread::msleep(50);
	}
	//ui->rotator_log->insertPlainText(QString("turn to the position! %1\n").arg(ui->current_azimuth->text())
}

void Rotator::rotator_set_pitch(double pitch) {
	RotatorProtocol rp;
	rp.set_target_angle(0, (int)(pitch * 100), PITCH);
	rotator_send_cmd(rp.get_bitstring());
}

void Rotator::rotator_set_azimuth(double azimuth) {
	RotatorProtocol rp;
	rp.set_target_angle((int)(azimuth * 100), 0, AZIMUTH);
	rotator_send_cmd(rp.get_bitstring());
}
