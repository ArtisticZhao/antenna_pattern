#include "Mission.h"
#include <QMessageBox>
#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>
#include "MotorCtrl.h"

QT_CHARTS_USE_NAMESPACE

// iterator like python range. useful.
struct double_inc_iterator : std::iterator<std::forward_iterator_tag, double> {
	double_inc_iterator(double initial, double inc = 1.0) : _value(initial), _inc(inc) {}
	value_type operator*() const { return _value; }
	double_inc_iterator& operator++() { _value += _inc; return *this; }

	bool operator==(double_inc_iterator const& r) const { return _value >= r._value; }
	bool operator!=(double_inc_iterator const& r) const { return !(*this == r); }

	value_type _value;
	value_type _inc;
};


Mission::Mission(MissonType type, 
	bool polar_enable, double polar_start, double polar_stop, 
	double polar_step, bool rotator_enable, double rotator_start, 
	double rotator_stop, double rotator_step, 
	N9918a* n9918a, PolarMotor* polar_motor, Rotator* rotator)
	:type(type), 
	polar_motor(polar_motor), polar_enable(polar_enable), polar_start(polar_start), polar_stop(polar_stop), polar_step(polar_step),
	rotator(rotator), rotator_enable(rotator_enable), rotator_start(rotator_start), rotator_stop(rotator_stop), rotator_step(rotator_step),
	n9918a(n9918a){
	stop_signal = false;
	pattern = nullptr;
	spectrum = nullptr;
	file_obj = nullptr;
	power_max = 0;
	power_min = 0;
	pattern_data = new QSplineSeries();
	spectrum_data = new QLineSeries();

	mission_num = 0;
	mission_count = 0;
}

void Mission::mission_start(QString file_full) {
	/*if (!is_all_ready()) {
		return;
	}*/
	bool res = create_folder(file_full);
	if (!res) return;
	emit status_changed(true);  // signed start
	// generate rotator angle vector to iteration
	std::vector<double> rotator_v(double_inc_iterator(rotator_start, rotator_step), double_inc_iterator(rotator_stop));
	// generate polar motor angle vector to iteration
	std::vector<double> polar_v(double_inc_iterator(polar_start, polar_step), double_inc_iterator(polar_stop));
	double inner_angle, outter_angle;

	// pre-config is not valid, measure once.
	if ((!polar_enable && !rotator_enable)
		|| (type == MissonType::RadiationPattern && !rotator_enable)
		|| (type == MissonType::Polarization && !polar_enable)) {
		
		qDebug() << "measure";
		double mesure_max;
		draw_spectrum(n9918a->measure_power(&mesure_max));

		// for test
		/*new_pattern();
		for (int i = 0; i < 360; i++) {
			QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
			draw_pattern_add_point((double)i, 1.0);
			QThread::msleep(50);
		}
		save_pattern_to_file("test");*/

		// test spectrum chart
		/*QLineSeries* qs = new QLineSeries();
		for (int i=0; i<100; i++) {
			qs->append(i, i);
		}
		draw_spectrum(qs);*/
	}

	// normal measure.
	// �߼���������: ���۲�������ͼ���Ǽ���ͼ, ����Ҫ����˫��ѭ��ִ��, ��ôֻ��Ҫȷ������ѭ�������ݼ���
	std::vector<double>* innerloop;
	std::vector<double>* outterloop;
	MotorCtrl* inner_motor;
	MotorCtrl* outter_motor;
	QString save_filename;
	// ��������ѭ��������
	if (type == MissonType::RadiationPattern) {
		// ��������ͼ, ���rotator�ĽǶ�����ѭ��, polar �ĽǶ�����ѭ��
		innerloop = &rotator_v;
		outterloop = &polar_v;
		// ���常��ָ��
		inner_motor = rotator;
		outter_motor = polar_motor;
		if (!polar_enable) {
			// ֻ�⵱ǰ�����Ƕȵ� ����ͼ.
			outterloop = nullptr;
			save_filename = QString("RadiationPattern_once");
			mission_num = innerloop->size();
		} else {
			save_filename = QString("RadiationPattern_at_polar_");
			mission_num = outterloop->size() * innerloop->size();
		}
	} else if (type == MissonType::Polarization) {
		// ��������ͼ, ���rotator�ĽǶ�����ѭ��, polar �ĽǶ�����ѭ��
		outterloop = &rotator_v;
		innerloop = &polar_v;
		outter_motor = rotator;
		inner_motor = polar_motor;
		if (!rotator_enable) {
			// ֻ�⵱ǰ����Ƕȵ� ����ͼ.
			outterloop = nullptr;
			save_filename = QString("Polarization_once");
			mission_num = innerloop->size();
		} else {
			save_filename = QString("Polarization_at_direction_");
			mission_num = outterloop->size() * innerloop->size();
		}
	} else if (type == MissonType::RadiationPattern3D) {
		 // 3d ����ͼ����
		// ��ѭ���Ƿ�λ  ��ѭ���Ǹ���
		outterloop = &polar_v;
		innerloop = &rotator_v;
		outter_motor = rotator;
		inner_motor = rotator;
		save_filename = QString("RadiationPattern3D_at_azimuth_");
		mission_num = outterloop->size() * innerloop->size();
	}

	// ��ʼloop
	if (outterloop == nullptr) {
		// ֻ�����˵������
		measure_single_loop(innerloop, inner_motor, save_filename);
		if (stop_signal) {
			emit status_changed(false);
			return;
		}
		QApplication::beep();  // ������ʾ��
	} else {
		// �����������
		outter_motor->turn_to_zero();
		foreach(outter_angle, *outterloop) {
			outter_motor->turn_to(outter_angle);
			measure_single_loop(innerloop, inner_motor, QString("%1%2").arg(save_filename).arg(QString::number(outter_angle)));
			if (stop_signal) {
				emit status_changed(false);
				return;
			}
		}
		QApplication::beep();  // ������ʾ��
	}

	emit status_changed(false);
}


void Mission::mission_stop() {
	emit logging(LogLevel::Warnning, "User signed mission stop signal.");
	stop_signal = true;
}


void Mission::measure_single_loop(std::vector<double>* innerloop, MotorCtrl* inner_motor, QString save_filename) {
	double inner_angle;
	if (type == MissonType::RadiationPattern3D) {
		inner_motor->turn_pitch_to(0);
	}
	else {
		inner_motor->turn_to_zero();
	}
	
	create_result_file(save_filename);
	new_pattern();
	foreach(inner_angle, *innerloop) {
		if (type == MissonType::RadiationPattern3D) {
			inner_motor->turn_pitch_to(inner_angle);
		}
		else {
			inner_motor->turn_to(inner_angle);
		}
		
		double measure_max;
		draw_spectrum(n9918a->measure_power(&measure_max));
		qDebug() << measure_max;
		draw_pattern_add_point(inner_angle, measure_max);
		QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
		append_result(inner_angle, n9918a->return_last_measure_data());
		mission_count++;
		emit process_changed(mission_count * 100 / mission_num);  //update process.
		if (stop_signal) {
			save_result_to_file();  // ��ǰ����ʱ���浽�ļ�,���ͷ��ļ���Դ
			return;
		}
	}
	//if (innerloop->at(0) == 0 && innerloop->back() == 360) {
	//	draw_pattern_add_point(360, pattern_data->at(0).y());  // ���ͼ��
	//}
	
	// save an file, save pattern picture.
	save_result_to_file();
	save_pattern_to_file(save_filename);
	emit logging(LogLevel::Info, "Single loop finish! ----------------------------------------------");
}

void Mission::init_draw() {
	// chart will auto connect with the series data,
	// so create chart, bind the series data and just modify the data.
	// No more recreate the chart!!
	// the child data "lineseries" will delete when old_chart delete!!

	// spectrum
	QChart* chart_spectrum = new QChart();
	chart_spectrum->legend()->hide();
	chart_spectrum->addSeries(spectrum_data);
	chart_spectrum->createDefaultAxes();
	spectrum->setChart(chart_spectrum);

	// pattern
	QChart* chart_pattern = new QPolarChart();
	chart_pattern->legend()->hide();
	chart_pattern->addSeries(pattern_data);
	chart_pattern->createDefaultAxes();
	// set axis range to 0 360
	QList<QAbstractAxis*> axisList = chart_pattern->axes();
	axisList.at(0)->setRange(0, 360);
	pattern->setChart(chart_pattern);
}

void Mission::draw_spectrum(QLineSeries* lineseries) {
	QChart* old_chart = spectrum->chart();
	QChart* c = new QChart();
	c->legend()->hide();
	c->addSeries(lineseries);
	c->createDefaultAxes();  //auto scale to added data.
	spectrum->setChart(c);
	if (old_chart != nullptr) {
		delete old_chart;
	}
}

void Mission::new_pattern() {
	power_max = -9999999;
	power_min = 9999999;
	pattern_data->clear();
	QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
	pattern->repaint();
}

void Mission::draw_pattern_add_point(double angle, double max_power) {
	// add measure point to pattern data
	pattern_data->append(angle, max_power);

	// update axis range.
	if (max_power > power_max) {
		power_max = max_power;
	}
	if (max_power < power_min) {
		power_min = max_power;
	}
	QList<QAbstractAxis*> axisList = pattern->chart()->axes();
	axisList.at(1)->setRange(power_min<0?power_min*1.2:power_min*0.8, 
		power_max<0?power_max*0.8:power_max*1.2);

	QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
}

bool Mission::create_folder(QString file_full) {
	// Process file name
	QFileInfo qfi = QFileInfo(file_full);
	QString filename = qfi.completeBaseName();
	QString filepath = qfi.absolutePath();

	// filename_type_startfreq_stopfreq
	QString save_dir_name = QString("%1_%2_%3_%4")
		.arg(filename)
		.arg((type == MissonType::Polarization) ? "Polarization" : (type == MissonType::RadiationPattern) ? "RadiationPattern" : "RadiationPattern3D")
		.arg(n9918a->start_freq).arg(n9918a->stop_freq);
	QDir dir(QDir::cleanPath(filepath + QDir::separator() + save_dir_name));
	qDebug() << dir.absolutePath();
	if (dir.exists()) {
		qDebug() << "dir exists";
		// file exist.
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(nullptr, QStringLiteral("�ļ��д���"), QStringLiteral("�ļ���%1 �Ѿ����ڣ��Ƿ�ɾ��ȫ������?").arg(save_dir_name),
			QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::No) {
			emit logging(LogLevel::Info, "User cancel");
			return true;
		} else {
			// delete and create
			bool res = dir.removeRecursively();
			if (!res) {
				emit logging(LogLevel::Error, QString("Remove folder %1 failed!").arg(save_dir_name));
				return res;
			}
		}
	}
	// create new folder
	bool res = QDir().mkdir(dir.absolutePath());
	if (!res) {
		emit logging(LogLevel::Error, QString("Create folder %1 failed!").arg(save_dir_name));
		return res;
	}
	emit logging(LogLevel::Info, QString("Measure result will save to %1!").arg(dir.absolutePath()));
	this->filepath = dir.absolutePath();
	return true;
}

bool Mission::create_result_file(QString filename) {
	QDir dir(QDir::cleanPath(filepath + QDir::separator() + filename));
	QString file_full = QString("%1.csv").arg(dir.absolutePath());
	qDebug() << file_full;
	file_obj = new QFile(file_full);
	if (file_obj->exists()) {
		emit logging(LogLevel::Warnning, QString("%1 file exists! overwrite...").arg(file_full));
		QFile fileTemp(file_full);
		fileTemp.remove();
	}
	if (!file_obj->open(QIODevice::ReadWrite)) {
		// file open failed
		emit logging(LogLevel::Error, QString("creat file %1 failed!").arg(file_full));
		return false;
	}
	emit logging(LogLevel::Info, QString("Saving result to %1").arg(file_full));
	// empty old data
	save_data.clear();
	save_data.append(QString("current angle, data(dBm)\n"));
}

void Mission::append_result(double current_angle, QString n9918_result) {
	save_data.append(QString("%1, %2\n").arg(QString::number(current_angle)).arg(n9918_result));
}

bool Mission::save_result_to_file() {
	
	foreach(QString str , save_data) {
		file_obj->write(str.toUtf8(), str.length());
	}
	file_obj->close();
	delete file_obj;
	return true;
}

void Mission::save_pattern_to_file(QString filename) {
	QScreen* screen = QGuiApplication::primaryScreen();
	QPixmap p = screen->grabWindow(pattern->winId());
	QImage image = p.toImage();
	QDir dir(QDir::cleanPath(filepath + QDir::separator() + filename));
	QString file_full = QString("%1.png").arg(dir.absolutePath());
	qDebug() << file_full;
	file_obj = new QFile(file_full);
	if (file_obj->exists()) {
		emit logging(LogLevel::Warnning, QString("%1 file exists! overwrite...").arg(file_full));
		QFile fileTemp(file_full);
		fileTemp.remove();
	}
	image.save(file_full);
	emit logging(LogLevel::Info, QString("pattern image save to %1").arg(file_full));
}

