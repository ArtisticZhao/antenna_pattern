#include "N9918a.h"
using namespace QtCharts;

N9918a::N9918a()
	:telnet(this),
	cmd_lock(false){
	deviceOK = DISCONNECTED;
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
	send_cmd("*CLS");
	send_cmd("*IDN?");
	send_cmd("SYST:PRES;*OPC?");
	send_cmd("INST:SEL 'SA';*OPC?");
	send_cmd(QString("SENS:SWE:POIN %1").arg(sample_points));
	send_cmd(QString("SENS:FREQ:START %1").arg(start_freq));
	send_cmd(QString("SENS:FREQ:STOP %1").arg(stop_freq));
}

void N9918a::send_cmd(const QString& cmd) {
	telnet.sendData(cmd.toUtf8());
	telnet.sendData("\n");
	//te_n9918a_log->insertPlainText(QString("[CMD] %1\n").arg(QString(cmd.toUtf8())));
	// 当处理查询语句之后应该等待返回消息再发送下一条
	if (cmd.indexOf('?') != -1) {
		cmd_lock = true;
		int timeout = 100;
		while (cmd_lock) {
			// 防止等待相应导致主线程假死
			QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
			timeout--;
			if (timeout == 0) {
				emit logging(LOG_ERROR, QString("N9918A timeout! Last cmd: %1").arg(cmd));
				break;
			}
			QThread::msleep(50);
		}
	}
}

QT_CHARTS_NAMESPACE::QLineSeries* N9918a::measure_power() {
	send_cmd("INIT:CONT OFF;*OPC?");
	send_cmd("INIT:IMM;*OPC?");
	send_cmd("TRACE:DATA?");
	QStringList list = last_anser.split(",");
	double max = -1e100;
	int max_index = -1;
	double min = 1e100;
	int index = -1;

	QLineSeries* lineseries = new QLineSeries();  //频谱的数据集

	foreach(QString num, list) {
		index++;
		double temp = num.toDouble();
		//lineseries->append(xaxis->at(index), temp);
		if (max < temp) {
			max = temp;
			max_index = index;
		}
		if (min > temp) {
			min = temp;
		}
	}

	emit measure_updated(max, min, max_index);
	return lineseries;
}

void N9918a::msg_callback(const char* msg, int count){
	last_anser = QByteArray(msg, count);
	cmd_lock = false;
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
		deviceOK = DISCONNECTED;
		break;
	case QAbstractSocket::HostLookupState:
		deviceOK = CONNECTING;
		break;
	case QAbstractSocket::ConnectingState:
		deviceOK = CONNECTING;
		break;
	case QAbstractSocket::ConnectedState:
		deviceOK = CONNECTED;
		break;
	case QAbstractSocket::BoundState:
		deviceOK = CONNECTED;
		break;
	case QAbstractSocket::ListeningState:
		deviceOK = CONNECTED;
		break;
	case QAbstractSocket::ClosingState:
		deviceOK = DISCONNECTED;
		break;
	}
	emit status_changed(deviceOK);
}
