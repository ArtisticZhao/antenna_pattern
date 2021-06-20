#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QTelnet.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void set_status_text(const QString &n9918a, const QString &rotator);
    void init_n9918a();
    void send_cmd_9918a(const QString &cmd);
    void measure_power();

private slots:
    void on_lan_connect_clicked();
    void on_state_changed(QAbstractSocket::SocketState s);

    void on_start_test_clicked();

    void on_n9918_log_textChanged();

public slots:
    void add_n9918a_log(const char *msg, int count);

private:
    Ui::MainWindow *ui;
    QTelnet telnet;
    QString status_n9918a, status_rotator;
    bool cmd_lock;  // 命令锁，如果命令是带有？查询语句，需要等待结果返回


};
#endif // MAINWINDOW_H
