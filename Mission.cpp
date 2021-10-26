#include "Mission.h"
#include <QMessageBox>
#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>


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
	pattern = nullptr;
	spectrum = nullptr;
	pattern_data = new QSplineSeries();
	spectrum_data = new QLineSeries();
}

void Mission::mission_start(QString file_full) {
	
	/*if (!is_all_ready()) {
		return;
	}*/
	bool res = create_folder(file_full);
	if (!res) return;

	// generate rotator angle vector to iteration
	std::vector<double> rotator_v(double_inc_iterator(rotator_start, rotator_step), double_inc_iterator(rotator_stop));
	// generate polar motor angle vector to iteration
	std::vector<double> polar_v(double_inc_iterator(polar_start, polar_step), double_inc_iterator(polar_stop));
	double pre_rotator_angle, pre_polar_angle;

	if ((!polar_enable && !rotator_enable) 
		|| (type == MissonType::RadiationPattern && !rotator_enable)
		|| (type == MissonType::Polarization && !polar_enable)){
		// measure once.
		qDebug() << "measure";
		draw_spectrum(n9918a->measure_power());
		
		// for test
		/*new_pattern();
		for (int i = 0; i < 360; i++) {
			QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
			draw_pattern_add_point((double)i, 1.0);
			QThread::msleep(50);
		}*/
	}
	else if (type == MissonType::RadiationPattern) {
		if (!polar_enable) {
			rotator->turn_to_zero();
			emit logging(INFO, QString("rotator set to ZERO"));
			// create result file.
			create_result_file(QString("RadiationPattern_once"));
			foreach(pre_rotator_angle, rotator_v) {
				rotator->turn_to(pre_rotator_angle);
				emit logging(INFO, QString("rotator to %1").arg(pre_rotator_angle));
				draw_spectrum(n9918a->measure_power());
				// save one line.
				append_result(pre_rotator_angle, n9918a->return_last_measure_data());
			}
			// TODO save an file, save pattern picture.
			save_result_to_file();
		} else {
			polar_motor->turn_to_zero();
			emit logging(INFO, QString("polar motor set to ZERO"));
			foreach(pre_polar_angle, polar_v) {
				polar_motor->turn_to(pre_polar_angle);
				emit logging(INFO, QString("polar motor to %1").arg(pre_polar_angle));
				rotator->turn_to_zero();
				emit logging(INFO, QString("rotator set to ZERO"));
				// create result file.
				create_result_file(QString("RadiationPattern_polar_%1").arg(QString::number(pre_polar_angle)));
				foreach(pre_rotator_angle, rotator_v) {
					rotator->turn_to(pre_rotator_angle);
					emit logging(INFO, QString("rotator to %1").arg(pre_rotator_angle));
					draw_spectrum(n9918a->measure_power());
					// save one line.
					append_result(pre_rotator_angle, n9918a->return_last_measure_data());
				}
				// TODO save an file, save pattern picture.
				save_result_to_file();
			}
		}
	}
	else if (type == MissonType::Polarization) {
		if (!rotator_enable) {
			polar_motor->turn_to_zero();
			emit logging(INFO, QString("polar motor set to ZERO"));
			// TODO create result file
			foreach(pre_polar_angle, polar_v) {
				polar_motor->turn_to(pre_polar_angle);
				emit logging(INFO, QString("polar motor to %1").arg(pre_polar_angle));
				draw_spectrum(n9918a->measure_power());
				// save one line.
				append_result(pre_polar_angle, n9918a->return_last_measure_data());
			}
			// TODO save an file, save pattern picture.
			save_result_to_file();
		} else {
			rotator->turn_to_zero();
			emit logging(INFO, QString("rotator set to ZERO"));
			foreach(pre_rotator_angle, rotator_v){
				rotator->turn_to(pre_rotator_angle);
				emit logging(INFO, QString("rotator to %1").arg(pre_rotator_angle));
				rotator->turn_to_zero();
				emit logging(INFO, QString("rotator set to ZERO"));
				// TODO create result file
				foreach(pre_polar_angle, polar_v) {
					polar_motor->turn_to(pre_polar_angle);
					emit logging(INFO, QString("polar motor to %1").arg(pre_polar_angle));
					draw_spectrum(n9918a->measure_power());
					// save one line.
					append_result(pre_polar_angle, n9918a->return_last_measure_data());
				}
				// TODO save an file, save pattern picture.
				save_result_to_file();
			}
		}
	}
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
	spectrum_data = lineseries;
	delete spectrum->chart()->axisX();
	delete spectrum->chart()->axisY();
	spectrum->repaint();
	delete spectrum->chart()->series()[0];  // delete old series.
	spectrum->chart()->addSeries(spectrum_data);
	
	spectrum->chart()->createDefaultAxes();
}

void Mission::new_pattern() {
	pattern_data->clear();
}

void Mission::draw_pattern_add_point(double angle, double max_power) {
	// add measure point to pattern data
	pattern_data->append(angle, max_power);
}

bool Mission::create_folder(QString file_full) {
	// Process file name
	QFileInfo qfi = QFileInfo(file_full);
	QString filename = qfi.completeBaseName();
	QString filepath = qfi.absolutePath();

	// filename_type_startfreq_stopfreq
	QString save_dir_name = QString("%1_%2_%3_%4")
		.arg(filename)
		.arg((type == MissonType::Polarization) ? "Polarization" : "RadiationPattern")
		.arg(n9918a->start_freq).arg(n9918a->stop_freq);
	QDir dir(QDir::cleanPath(filepath + QDir::separator() + save_dir_name));
	qDebug() << dir.absolutePath();
	if (dir.exists()) {
		qDebug() << "dir exists";
		// file exist.
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(nullptr, QStringLiteral("文件夹存在"), QStringLiteral("文件夹%1 已经存在，是否删除全部内容?").arg(save_dir_name),
			QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::No) {
			emit logging(INFO, "User cancel");
			return true;
		} else {
			// delete and create
			bool res = dir.removeRecursively();
			if (!res) {
				emit logging(LOG_ERROR, QString("Remove folder %1 failed!").arg(save_dir_name));
				return res;
			}
		}
	}
	// create new folder
	bool res = QDir().mkdir(dir.absolutePath());
	if (!res) {
		emit logging(LOG_ERROR, QString("Create folder %1 failed!").arg(save_dir_name));
		return res;
	}
	emit logging(INFO, QString("Measure result will save to %1!").arg(dir.absolutePath()));
	this->filepath = dir.absolutePath();
	return true;
}

bool Mission::create_result_file(QString filename) {
	QDir dir(QDir::cleanPath(filepath + QDir::separator() + filename));
	QString file_full = QString("%1.csv").arg(dir.absolutePath());
	qDebug() << file_full;
	file_obj = new QFile(file_full);
	if (file_obj->exists()) {
		emit logging(WARNNING, QString("%1 file exists! overwrite...").arg(file_full));
		QFile fileTemp(file_full);
		fileTemp.remove();
	}
	if (!file_obj->open(QIODevice::ReadWrite)) {
		// file open failed
		emit logging(LOG_ERROR, QString("creat file %1 failed!").arg(file_full));
		return false;
	}
	emit logging(INFO, QString("Saving result to %1").arg(file_full));
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

