#include <QThread>
#include <QCoreApplication>
#include "Rotator.h"
#include "rotatorprotocol.h"

Rotator::Rotator() {
	// ��ʼ�������ڶ�ʱ��
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
		// ��ʼ������
		com = new QextSerialPort(com_port, QextSerialPort::Polling);
		comOk = com->open(QIODevice::ReadWrite);
		if (comOk) {
			//��ջ�����
			com->flush();
			//���ò�����
			com->setBaudRate(BAUD115200);
			//��������λ
			com->setDataBits(DATA_8);
			//����У��λ
			com->setParity(PAR_NONE);
			//����ֹͣλ
			com->setStopBits(STOP_1);
			com->setFlowControl(FLOW_OFF);
			com->setTimeout(10);

			//��ȡ����
			timerRead->start();
			emit status_changed();
		}
	}
}

void Rotator::disconnect() {
	if (comOk) {
		// �Ͽ�����
		timerRead->stop();
		comOk = false;
		com->close();
		com->deleteLater();
		emit status_changed();
	}
}

void Rotator::turn_to(double azimuth) {
	RotatorProtocol rp;
	rp.set_target_angle((int)(azimuth * 100), 0, AZIMUTH);
	send_cmd(rp.get_bitstring());
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
			emit logging(WARNNING, "Rotator timeout!");
			break;
		}
		QThread::msleep(50);
	}
	emit logging(INFO, QString("turn to the position! %1\n").arg(current_azimuth));
}

void Rotator::set_pitch(double pitch) {
	RotatorProtocol rp;
	rp.set_target_angle(0, (int)(pitch * 100), PITCH);
	send_cmd(rp.get_bitstring());
}

void Rotator::set_azimuth(double azimuth) {
	RotatorProtocol rp;
	rp.set_target_angle((int)(azimuth * 100), 0, AZIMUTH);
	send_cmd(rp.get_bitstring());
}

void Rotator::send_cmd(const QByteArray& cmd) {
	if (comOk) {
		com->write(cmd);
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
		// ����֡ͷ, �����Ƕ�
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