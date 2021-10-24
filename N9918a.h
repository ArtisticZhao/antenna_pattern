#pragma once
#include <QObject>
#include <QLineEdit>
#include <QTextEdit>
#include <QCoreApplication>
#include <QThread>
#include <QtCharts>
#include <QTelnet.h>

QT_CHARTS_USE_NAMESPACE

class N9918a : public QObject{
public:
	N9918a(QTextEdit* te_n9918a_log);
	~N9918a();
private:
	QTelnet telnet;
	QString status_n9918a;
	bool cmd_lock;  // 命令锁，如果命令是带有？查询语句，需要等待结果返回
	QString n9918a_last_anser;

	// parent Qt widgets
	QTextEdit* te_n9918a_log;
	
	void n9918a_connect(QString ip_addr);
	void n9918a_init(QString sample_points, QString start_freq, QString stop_freq);
	void n9918a_send_cmd(const QString& cmd);
	//void n9918a_freq_linespace();  // 生成图像横坐标数据
	QLineSeries* n9918a_measure_power();
	void n9918a_msg_callback(const char* msg, int count);
	void n9918a_disconnect();
};

