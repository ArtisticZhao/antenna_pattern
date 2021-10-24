#include "N9918a.h"
using namespace QtCharts;

N9918a::N9918a(QTextEdit* te_n9918a_log)
	:telnet(this),
	te_n9918a_log(te_n9918a_log),
	cmd_lock(false){

	//connect(&telnet, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(on_state_changed(QAbstractSocket::SocketState)));
	connect(&telnet, SIGNAL(newData(const char*, int)), this, SLOT(n9918a_msg_callback(const char*, int)));
}

N9918a::~N9918a() {
	n9918a_disconnect();
}

void N9918a::n9918a_connect(QString ip_addr) {
	if (!telnet.isConnected()) {
		telnet.connectToHost(ip_addr, 5025);
	}
	
}

void N9918a::n9918a_init(QString sample_points, QString start_freq, QString stop_freq) {
	n9918a_send_cmd("*CLS");
	n9918a_send_cmd("*IDN?");
	n9918a_send_cmd("SYST:PRES;*OPC?");
	n9918a_send_cmd("INST:SEL 'SA';*OPC?");
	n9918a_send_cmd(QString("SENS:SWE:POIN %1").arg(sample_points));
	n9918a_send_cmd(QString("SENS:FREQ:START %1").arg(start_freq));
	n9918a_send_cmd(QString("SENS:FREQ:STOP %1").arg(stop_freq));
}

void N9918a::n9918a_send_cmd(const QString& cmd) {
	telnet.sendData(cmd.toUtf8());
	telnet.sendData("\n");
	te_n9918a_log->insertPlainText(QString("[CMD] %1\n").arg(QString(cmd.toUtf8())));
	// 当处理查询语句之后应该等待返回消息再发送下一条
	if (cmd.indexOf('?') != -1) {
		cmd_lock = true;
		int timeout = 100;
		while (cmd_lock) {
			// 防止等待相应导致主线程假死
			QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
			timeout--;
			if (timeout == 0) {
				te_n9918a_log->insertPlainText("Timeout!\n");
				break;
			}
			QThread::msleep(50);
		}
	}
}

QT_CHARTS_NAMESPACE::QLineSeries* N9918a::n9918a_measure_power() {
	n9918a_send_cmd("INIT:CONT OFF;*OPC?");
	n9918a_send_cmd("INIT:IMM;*OPC?");
	n9918a_send_cmd("TRACE:DATA?");
	QStringList list = n9918a_last_anser.split(",");
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
	return lineseries;
}

void N9918a::n9918a_msg_callback(const char* msg, int count){
	n9918a_last_anser = QByteArray(msg, count);
	te_n9918a_log->insertPlainText(n9918a_last_anser);
	te_n9918a_log->insertPlainText("\n");
	cmd_lock = false;
}

void N9918a::n9918a_disconnect() {
	if (telnet.isConnected()) {
		telnet.disconnectFromHost();
	}
}
