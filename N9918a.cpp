#include "N9918a.h"
using namespace QtCharts;

N9918a::N9918a()
	:telnet(this),
	cmd_lock(false){
	deviceOK = DevStatus::disconnected;
	xaxis = nullptr;
	sample_points = 0;
	measure_lock = false;
	measure_data_counter = 0;
	connect(&telnet, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(on_state_changed(QAbstractSocket::SocketState)));
	connect(&telnet, SIGNAL(newData(const char*, int)), this, SLOT(msg_callback(const char*, int)));
}

N9918a::~N9918a() {
	disconnect();
}

void N9918a::connectToN9918a(QString ip_addr) {
	if (!telnet.isConnected()) {
		telnet.connectToHost(ip_addr, 5025);
	}
}

void N9918a::init(QString sample_points, QString start_freq, QString stop_freq) {
	this->start_freq = start_freq;
	this->stop_freq = stop_freq;
	this->sample_points = sample_points.toInt();
	send_cmd("*CLS");
	send_cmd("*IDN?");
	send_cmd("SYST:PRES;*OPC?");
	send_cmd("INST:SEL 'SA';*OPC?");
	send_cmd(QString("SENS:SWE:POIN %1").arg(sample_points));
	send_cmd(QString("SENS:FREQ:START %1").arg(start_freq));
	send_cmd(QString("SENS:FREQ:STOP %1").arg(stop_freq));
	// generate x-axis
	generate_freq_linespace(sample_points, start_freq, stop_freq);
	emit logging(LogLevel::Info, QString("N9918A set start freq at %1; stop freq at %2; sample point is %3").arg(start_freq).arg(stop_freq).arg(sample_points));
}

QString N9918a::return_last_measure_data() {
	return last_anser;
}

void N9918a::send_cmd(const QString& cmd) {
	if (deviceOK != DevStatus::connected) {
		emit logging(LogLevel::Error, QString("N9918A not ready! Last cmd: %1").arg(cmd));
		return;
	}
	telnet.sendData(cmd.toUtf8());
	telnet.sendData("\n");
	// '?' means this cmd the N9918a will respond.
	if (cmd.indexOf('?') != -1) {
		cmd_lock = true;
		int timeout = 100;
		while (cmd_lock) {
			// so raise cmd_lock and wait the N9918 respond.
			QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
			timeout--;
			if (timeout == 0) {
				emit logging(LogLevel::Error, QString("N9918A timeout! Last cmd: %1").arg(cmd));
				break;
			}
			QThread::msleep(50);
		}
	}
}

void N9918a::generate_freq_linespace(QString sample_points_str, QString start_freq_str, QString stop_freq_str) {
	if (xaxis != nullptr) {
		delete xaxis;
	}

	double start_freq = start_freq_str.toDouble();
	double stop_freq = stop_freq_str.toDouble();
	int sample_points = sample_points_str.toInt();
	xaxis = new QVector<double>();
	double step_freq = (stop_freq - start_freq) / sample_points;
	for (int i = 0; i < sample_points; i++) {
		xaxis->push_back(start_freq + i * step_freq);
	}
}

QLineSeries* N9918a::measure_power(double* power_max) {
	send_cmd("INIT:CONT OFF;*OPC?");
	send_cmd("INIT:IMM;*OPC?");
	last_anser.clear();
	measure_lock = true; // lock
	measure_data_counter = 0;
	send_cmd("TRACE:DATA?");
	QStringList list = last_anser.split(",");
	double max = -1e100;
	int max_index = -1;
	double min = 1e100;
	int index = -1;

	QLineSeries* lineseries = new QLineSeries();  // QLineSeries is for QChart data struct. which is a series of (x, y).
												  // and this lineseries of data will delete by QChart!
	foreach(QString num, list) {
		index++;
		double temp = num.toDouble();
		lineseries->append(xaxis->at(index), temp);
		if (max < temp) {
			max = temp;
			max_index = index;
		}
		if (min > temp) {
			min = temp;
		}
	}
	*power_max = max;
	emit measure_updated(max, min, xaxis->at(max_index));
	qDebug() << max << max_index;
	return lineseries;
}

void N9918a::msg_callback(const char* msg, int count){
	if (measure_lock) {
		QString tmp_str = QByteArray(msg, count);
		QStringList list = tmp_str.split(",");
		last_anser.append(tmp_str);
		measure_data_counter += list.size();
		if (measure_data_counter >= sample_points) {
			qDebug() << "full last: " << last_anser;
			measure_lock = false;
			cmd_lock = false;
		}
		else {
			qDebug() << "last: " << last_anser;
			qDebug() << measure_data_counter;
		}
	}
	else {
		last_anser = QByteArray(msg, count);
		cmd_lock = false;
	}
}

void N9918a::disconnect() {
	if (telnet.isConnected()) {
		telnet.disconnectFromHost();
	}
}

void N9918a::on_state_changed(QAbstractSocket::SocketState s) {

	switch (s)
	{
	case QAbstractSocket::UnconnectedState:
		deviceOK = DevStatus::disconnected;
		break;
	case QAbstractSocket::HostLookupState:
		deviceOK = DevStatus::connecting;
		break;
	case QAbstractSocket::ConnectingState:
		deviceOK = DevStatus::connecting;
		break;
	case QAbstractSocket::ConnectedState:
		deviceOK = DevStatus::connected;
		break;
	case QAbstractSocket::BoundState:
		deviceOK = DevStatus::connected;
		break;
	case QAbstractSocket::ListeningState:
		deviceOK = DevStatus::connected;
		break;
	case QAbstractSocket::ClosingState:
		deviceOK = DevStatus::disconnected;
		break;
	}
	emit status_changed(deviceOK);
}
