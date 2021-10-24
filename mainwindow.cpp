#include <cmath>
#include <QThread>
#include <QFileDialog>
#include <QSerialPortInfo>
#include <QStandardPaths>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "rotatorprotocol.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    n9918a = nullptr;
    polar_motor = new PolarMotor();
    rotator = new Rotator();

    // 创建信号连接
    connect(ui->cb_polar_motor_com, SIGNAL(combo_box_showpopup(QComboBoxMoreSignal*)), this, SLOT(on_cb_com_showPopup(QComboBoxMoreSignal*)));
    connect(ui->cb_rotator_com, SIGNAL(combo_box_showpopup(QComboBoxMoreSignal*)), this, SLOT(on_cb_com_showPopup(QComboBoxMoreSignal*)));

    connect(polar_motor, SIGNAL(polor_motor_status_changed()), this, SLOT(on_polar_motor_status_changed()));
    connect(polar_motor, SIGNAL(polor_motor_update_angle(double)), this, SLOT(on_polar_motor_angle_updated(double)));
    connect(polar_motor, SIGNAL(polar_motor_logging(LOGLEVEL, QString)), this, SLOT(on_logging(LOGLEVEL, QString)));

//    //
//    plot.setPlotBackground (QBrush( Qt::red, Qt::SolidPattern ));
//    plot.setScale( QwtPolar::ScaleAzimuth, 0, 360, 30);
//    plot.setScale( QwtPolar::ScaleRadius, 0, 4, 2);

//    QwtPolarGrid grid;
//    grid.setFont (QFont("Times", 12, QFont::Bold));
//    grid.setPen ( QPen(Qt::blue, 1, Qt::DashDotLine) );
//    //grid.setAxisFont (QwtPolar::AxisLeft,  QFont("Times", 6));
//    grid.setAxisPen ( QwtPolar::AxisAzimuth, QPen(Qt::black, 1) );
//    //grid.setAxisPen ( QwtPolar::AxisLeft, QPen(Qt::black, 1) );
//    grid.showMinorGrid (QwtPolar::AxisLeft, true);
//    grid.showMinorGrid (QwtPolar::AxisRight, true);
//    grid.showMinorGrid (QwtPolar::AxisTop, true);
//    grid.showMinorGrid (QwtPolar::AxisBottom, true);
//    grid.showGrid (QwtPolar::AxisAzimuth, true);
//    grid.showGrid (QwtPolar::AxisLeft, true);
//    grid.showGrid (QwtPolar::AxisRight, true);
//    grid.showGrid (QwtPolar::AxisTop, true);
//    grid.showGrid (QwtPolar::AxisBottom, true);
//    grid.attach(&plot);
//    plot.resize(800,600);
//    plot.show();
    c = new QChart();
    pattern_data = new QSplineSeries();
    chart = new QPolarChart();
}

MainWindow::~MainWindow()
{
    delete n9918a;
    delete polar_motor;
    delete rotator;
    delete ui;
}

void MainWindow::on_logging(LOGLEVEL level, QString msg) {
	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_date = current_date_time.toString("hh:mm:ss.zzz");

    switch (level) {
    case INFO:
        ui->textEdit_log->append(QString("%1: [INFO] %2").arg(current_date).arg(msg));
        break;
    case WARNNING:
        ui->textEdit_log->append(QString("<font color=\"#FF9912\">%1: [WARNNING] %2</font> ").arg(current_date).arg(msg));
        break;
    case LOG_ERROR:
        ui->textEdit_log->append(QString("<font color=\"#FF0000\">%1: [ERROR] %2</font> ").arg(current_date).arg(msg));
        break;
    default:
        break;
    }
}

void MainWindow::on_cb_com_showPopup(QComboBoxMoreSignal* combo_box) {
	combo_box->clear();

	// 列出可用串口
	foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts()) {
		combo_box->addItem(info.portName());
	}
}

void MainWindow::on_pb_polar_motor_connect_clicked() {
    qDebug() << "on polar motor connect" << polar_motor->comOk;
    if (!polar_motor->comOk) {
        polar_motor->polar_motor_connect(ui->cb_polar_motor_com->currentText());
    }
    else {
        polar_motor->polar_motor_disconnect();
    }
    
}

void MainWindow::on_pb_polar_motor_set_angle_clicked() {
    qDebug() << "on polar motor set angle";
    polar_motor->polar_motor_turn(ui->le_polar_motor_cmd_angle->text().toDouble());
}

void MainWindow::on_pb_polar_motor_reset_angle_clicked() {
    polar_motor->polar_motor_reset();
}

void MainWindow::on_polar_motor_status_changed() {
    qDebug() << "on polar motor status changed" << polar_motor->comOk;
    if (polar_motor->comOk) {
        ui->pb_polar_motor_connect->setText(QStringLiteral("断开"));
        ui->cb_polar_motor_com->setEnabled(false);
        ui->pb_polar_motor_reset_angle->setEnabled(true);
        ui->pb_polar_motor_set_angle->setEnabled(true);
    }
    else {
        ui->pb_polar_motor_connect->setText(QStringLiteral("连接"));
        ui->cb_polar_motor_com->setEnabled(true);
		ui->pb_polar_motor_reset_angle->setEnabled(false);
		ui->pb_polar_motor_set_angle->setEnabled(false);
    }
}

void MainWindow::on_polar_motor_angle_updated(double angle) {
    ui->le_polar_motor_current_angle->setText(QString::number(angle));
}

//void MainWindow::on_state_changed(QAbstractSocket::SocketState s)
//{
//    switch( s )
//        {
//        case QAbstractSocket::UnconnectedState:
//            set_status_text("disconnected", "");
//            ui->lan_connect->setText(QStringLiteral("1-连接"));
//            ui->start_test->setEnabled(false);
//            ui->ip_addr->setEnabled(true);
//            break;
//        case QAbstractSocket::HostLookupState:
//            set_status_text("connecting", "");
//            ui->lan_connect->setText(QStringLiteral("断开"));
//            break;
//        case QAbstractSocket::ConnectingState:
//            set_status_text("connecting", "");
//            ui->lan_connect->setText(QStringLiteral("断开"));
//            break;
//        case QAbstractSocket::ConnectedState:
//            set_status_text("connected", "");
//            ui->lan_connect->setText(QStringLiteral("断开"));
//            ui->ip_addr->setEnabled(false);
//            break;
//        case QAbstractSocket::BoundState:
//            set_status_text("connected", "");
//            ui->lan_connect->setText(QStringLiteral("断开"));
//            break;
//        case QAbstractSocket::ListeningState:
//            set_status_text("connected", "");
//            ui->lan_connect->setText(QStringLiteral("断开"));
//            break;
//        case QAbstractSocket::ClosingState:
//            set_status_text("disconnected", "");
//            ui->lan_connect->setText(QStringLiteral("断开"));
//            break;
//    }
//}

//void MainWindow::set_status_text(const QString &n9918a, const QString &rotator)
//{
//    if (n9918a.length()!=0){
//        status_n9918a = n9918a;
//    }
//
//    if (rotator.length()!=0){
//        status_rotator = rotator;
//    }
//
//    ui->statusbar->showMessage(QString("N9918A: %1 | Rotator: %2").arg(status_n9918a).arg(status_rotator));
//
//    // 改变 开始按钮状态
//    if (telnet.isConnected() && comOk){
//        ui->start_test->setEnabled(true);
//    }else{
//        ui->start_test->setEnabled(false);
//    }
//}


//void MainWindow::on_lan_connect_clicked()
//{
//    //if (! telnet.isConnected()){
//    //    telnet.connectToHost(ui->ip_addr->text(), 5025);
//    //    ui->lan_connect->setText(QStringLiteral("断开"));
//    //}else{
//    //    telnet.disconnectFromHost();
//    //    ui->lan_connect->setText(QStringLiteral("1-连接"));
//    //}
//
//}









//void MainWindow::measure_power()
//{
//    
//    ui->power_max->setText(QString::number(max));
//    ui->power_min->setText(QString::number(min));
//    ui->peek_freq->setText(QString::number(max_index));
//
//    // 绘制当前频谱
//    c = new QChart();
//    c->legend()->hide();
//    c->addSeries(lineseries);
//    c->createDefaultAxes();
//    ui->spectrum->setChart(c);
//
//    draw_pattern(max);
//
//
//}

//void MainWindow::draw_pattern(double max){
//    // 添加测量点到方向图
//    if (ui->tabWidget->currentIndex() == 0){
//        pattern_data->append(ui->current_azimuth->text().toDouble(), max);
//    }else{
//        pattern_data->append(ui->le_current_angle->text().toDouble(), max);
//    }
//
//    chart = new QPolarChart();
//    chart->legend()->hide();
//    chart->addSeries(pattern_data);
//
//    chart->createDefaultAxes();
//    QList<QAbstractAxis *> axisList = chart->axes();
//    axisList.at(0)->setRange(0,360);
//
//    ui->pattern->setChart(chart);
//}

//void MainWindow::freq_linespace()
//{
//    if(xaxis->length()!=0){
//        xaxis->clear();
//    }
//
//    double start_freq = ui->start_freq->text().toDouble();
//    double stop_freq = ui->stop_freq->text().toDouble();
//    int sample_points = ui->sample_points->text().toInt();
//    xaxis = new QVector<double>();
//    double step_freq = (stop_freq - start_freq)/sample_points;
//    for (int i=0; i<sample_points; i++) {
//        xaxis->push_back(start_freq + i*step_freq);
//    }
//}


//QString MainWindow::select_save_file()
//{
//    QString fileName;
//    fileName = QFileDialog::getSaveFileName(this,
//            tr("Save file"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), tr("CSV (*.csv)"));
//
//    if (!fileName.isNull())
//    {
//        //fileName是文件名
//
//        return fileName;
//    }else{
//        //点的是取消
//
//        return fileName;
//    }
//}

//void MainWindow::set_n9918_config_enable(bool enable)
//{
//    if (enable){
//        ui->start_freq->setEnabled(true);
//        ui->stop_freq->setEnabled(true);
//        ui->sample_points->setEnabled(true);
//    }else{
//        ui->start_freq->setEnabled(false);
//        ui->stop_freq->setEnabled(false);
//        ui->sample_points->setEnabled(false);
//    }
//}

//void MainWindow::set_rotator_config_enable(bool enable) {
//	/*if (enable) {
//		ui->start_azimuth->setEnabled(true);
//		ui->stop_azimuth->setEnabled(true);
//		ui->step_azimuth->setEnabled(true);
//		ui->pitch->setEnabled(true);
//	}
//	else {
//		ui->start_azimuth->setEnabled(false);
//		ui->stop_azimuth->setEnabled(false);
//		ui->step_azimuth->setEnabled(false);
//		ui->pitch->setEnabled(false);
//	}*/
//}

//void MainWindow::on_process_enable(bool enable)
//{
//    if (enable){
//        // 禁用
//        on_process = true;
//        set_n9918_config_enable(false);
//        set_rotator_config_enable(false);
//        ui->start_test->setEnabled(false);
//        ui->com_connect->setEnabled(false);
//        ui->lan_connect->setEnabled(false);
//        ui->set_pitch->setEnabled(false);
//        ui->pb_set_azimuth->setEnabled(false);
//        ui->le_azimuth->setEnabled(false);
//    }else {
//        // 启用
//        on_process = false;
//        ui->start_test->setEnabled(true);
//        ui->com_connect->setEnabled(true);
//        ui->lan_connect->setEnabled(true);
//        ui->set_pitch->setEnabled(true);
//        ui->pb_set_azimuth->setEnabled(true);
//        ui->le_azimuth->setEnabled(true);
//        set_n9918_config_enable(true);
//        set_rotator_config_enable(true);
//    }
//}


//void MainWindow::on_start_test_clicked()
//{
//    on_process_enable(true);
//
//    // 选择文件保存路径
//    QString filename = select_save_file();
//    if(filename.length()==0){
//        // 用户取消选择
//        on_process_enable(false);
//        return;
//    }
//    QFile file(filename);
//    //判断文件是否存在
//    if(file.exists()){
//        QFile fileTemp(filename);
//        fileTemp.remove();
//    }else{
//    }
//    //已读写方式打开文件，
//    //如果文件不存在会自动创建文件
//    if(!file.open(QIODevice::ReadWrite)){
//        QMessageBox messageBox(QMessageBox::Warning,
//                               QStringLiteral("打开失败"), QStringLiteral("打开文件失败，请重新开始"),
//                               QMessageBox::Yes , NULL);
//        messageBox.exec();
//        on_process_enable(false);
//        return;
//    }else{
//    }
//
//    //if( ui->tabWidget->currentIndex() == 1){
//    //    // 天线极化测试
//    //    // 写入表头
//    //    QString content("current angle, data(dBm)\n");
//    //    file.write(content.toUtf8(),content.length());
//    //    ui->progressBar->setValue(0);
//    //    init_n9918a();
//    //    freq_linespace();
//    //    QSplineSeries* old_data = pattern_data;
//
//    //    pattern_data = new QSplineSeries();
//    //    old_data->deleteLater();                  // 删掉旧的方向图数据
//
//    //    double step_angle = ui->le_step_angle->text().toDouble();
//    //    double total_angle = ui->le_total_angle->text().toDouble();
//    //    int movement = (int)(total_angle/step_angle);
//    //    int move_count = 0;
//    //    double current_angle = 0;
//    //    ui->le_current_angle->setText(QString::number(current_angle));
//    //    for (; move_count<movement; move_count++ ) {
//    //        ui->progressBar->setValue((int)(((double)move_count)/movement*100));
//    //        // save data to csv
//    //        measure_power();
//    //        content = QString("%1, %2\n").arg(current_angle).arg(last_9918_anser);
//    //        file.write(content.toUtf8(),content.length());
//
//    //        turn_motor(step_angle);
//    //        current_angle += step_angle;
//    //        ui->le_current_angle->setText(QString::number(current_angle));
//    //        if(kill_process){
//    //            kill_process = false;
//    //            on_process_enable(false);
//    //            file.close();
//    //            return;
//    //        }
//    //    }
//
//    //    file.close();
//
//    //    ui->progressBar->setValue(100);
//    //    on_process_enable(false);
//    //    return;
//    //}
//
//    // 写入表头
//    QString content("set pitch, current pitch, set azimuth, current azimuth, data(dBm)\n");
//    file.write(content.toUtf8(),content.length());
//
//    ui->progressBar->setValue(0);
//	//init_n9918a();
//	//freq_linespace();
//    QSplineSeries* old_data = pattern_data;
//
//    pattern_data = new QSplineSeries();
//    old_data->deleteLater();                  // 删掉旧的方向图数据
//
//    //double start_azimuth = ui->start_azimuth->text().toDouble();
//    //double stop_azimuth = ui->stop_azimuth->text().toDouble();
//    //double step_azimuth = ui->step_azimuth->text().toDouble();
//    //int movement = (int)((stop_azimuth-start_azimuth)/step_azimuth);
//    //int move_count = 0;
//    //double set_azimuth = start_azimuth;
//    //for (; move_count<movement; move_count++ ) {
//    //    ui->progressBar->setValue((int)(((double)move_count)/movement*100));
//    //    turn_rotator(set_azimuth);
//    //    // Debug
//    //    ui->current_azimuth->setText(QString::number(set_azimuth));
//    //    if(kill_process){
//    //        kill_process = false;
//    //        on_process_enable(false);
//    //        file.close();
//    //        return;
//    //    }
//
//    //    // save data to csv
//    //    measure_power();
//    //    content = QString("%1, %2, %3, %4, %5\n").arg(ui->pitch->text()).arg(ui->current_pitch->text()).arg(set_azimuth).arg(ui->current_azimuth->text()).arg(last_9918_anser);
//    //    file.write(content.toUtf8(),content.length());
//
//    //    set_azimuth += step_azimuth;
//    //}
//    //if (set_azimuth != stop_azimuth){
//    //    set_azimuth = stop_azimuth;
//    //    turn_rotator(set_azimuth);
//    //    // save data to csv
//    //    measure_power();
//    //    content = QString("%1, %2, %3, %4, %5\n").arg(ui->pitch->text()).arg(ui->current_pitch->text()).arg(set_azimuth).arg(ui->current_azimuth->text()).arg(last_9918_anser);
//    //    file.write(content.toUtf8(),content.length());
//    //}
//    //file.close();
//
//    //ui->progressBar->setValue(100);
//    //on_process_enable(false);
//}
//
//
////void MainWindow::on_n9918_log_textChanged()
////{
////    ui->n9918_log->moveCursor(QTextCursor::End);
////}
//
//
//void MainWindow::on_com_connect_clicked()
//{
//    
//}
//
//
//
//
//void MainWindow::on_set_pitch_clicked()
//{
//
//}
//
//
//
//void MainWindow::on_rotator_log_textChanged()
//{
//    ui->rotator_log->moveCursor(QTextCursor::End);
//}
//
//
//void MainWindow::on_stop_test_clicked()
//{
//    kill_process = true;
//}
//
//void MainWindow::closeEvent(QCloseEvent* event)
//{
//    //当文档内容被修改时.
//    if (!on_process){
//        event->accept();
//    }else{
//        event->ignore();
//    }
//}
//
//
//void MainWindow::on_btn_refresh_com_clicked()
//{
//    refresh_cmd_port_list();
//}
//
//
//void MainWindow::on_pb_set_azimuth_clicked()
//{
//    double azimuth = ui->le_azimuth->text().toDouble();
//}
//
//
//void MainWindow::on_btn_refresh_com_motor_clicked()
//{
//    refresh_cmd_port_list();
//}
//
//
//void MainWindow::on_com_connect_motor_clicked()
//{
//   
//}
//
//
//
//void MainWindow::on_pb_turn_motor_clicked()
//{
//    QString angle = ui->le_motor_angle->text();
//}


