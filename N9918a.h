#pragma once
#include <QObject>
#include <QLineEdit>
#include <QTextEdit>
#include <QCoreApplication>
#include <QThread>
#include <QtCharts>
#include <QTelnet.h>
#include "logging.h"

QT_CHARTS_USE_NAMESPACE

enum DEV_STATUS { DISCONNECTED = 1, CONNECTING = 2, CONNECTED = 3 };

class N9918a : public QObject{
	Q_OBJECT
signals:
	void logging(LOGLEVEL level, QString msg);
	void measure_updated(double max_power, double min_power, int max_index);
	void status_changed(DEV_STATUS deviceOK);

public:
	DEV_STATUS deviceOK;
	N9918a();
	~N9918a();
	void connectToN9918a(QString ip_addr);
	void disconnect();


private:
	QTelnet telnet;
	bool cmd_lock;  // 命令锁，如果命令是带有？查询语句，需要等待结果返回
	QString last_anser;

	
	void init(QString sample_points, QString start_freq, QString stop_freq);
	void send_cmd(const QString& cmd);
	//void n9918a_freq_linespace();  // 生成图像横坐标数据
	QLineSeries* measure_power();
	void msg_callback(const char* msg, int count);
	

private slots:
	void on_state_changed(QAbstractSocket::SocketState s);
};

