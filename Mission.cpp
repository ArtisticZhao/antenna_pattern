#include "Mission.h"

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

void Mission::mission_start(QString filepath, QString filename) {
	/*if (!is_all_ready()) {
		return;
	}*/
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
			foreach(pre_rotator_angle, rotator_v) {
				rotator->turn_to(pre_rotator_angle);
				emit logging(INFO, QString("rotator to %1").arg(pre_rotator_angle));
				// measure
				// save one line.
			}
			// save an file, save pattern picture.
		} else {
			polar_motor->turn_to_zero();
			emit logging(INFO, QString("polar motor set to ZERO"));
			foreach(pre_polar_angle, polar_v) {
				polar_motor->turn_to(pre_polar_angle);
				emit logging(INFO, QString("polar motor to %1").arg(pre_polar_angle));
				rotator->turn_to_zero();
				emit logging(INFO, QString("rotator set to ZERO"));
				foreach(pre_rotator_angle, rotator_v) {
					rotator->turn_to(pre_rotator_angle);
					emit logging(INFO, QString("rotator to %1").arg(pre_rotator_angle));
					// measure
					// save one line.
				}
				// save an file, save pattern picture.
			}
		}
	}
	else if (type == MissonType::Polarization) {
		if (!rotator_enable) {
			polar_motor->turn_to_zero();
			emit logging(INFO, QString("polar motor set to ZERO"));
			foreach(pre_rotator_angle, rotator_v) {
				rotator->turn_to(pre_rotator_angle);
				emit logging(INFO, QString("rotator to %1").arg(pre_rotator_angle));
				// measure
				// save one line.
			}
			// save an file, save pattern picture.
		} else {
			rotator->turn_to_zero();
			emit logging(INFO, QString("rotator set to ZERO"));
			foreach(pre_rotator_angle, rotator_v){
				rotator->turn_to(pre_rotator_angle);
				emit logging(INFO, QString("rotator to %1").arg(pre_rotator_angle));
				rotator->turn_to_zero();
				emit logging(INFO, QString("rotator set to ZERO"));
				foreach(pre_polar_angle, polar_v) {
					polar_motor->turn_to(pre_polar_angle);
					emit logging(INFO, QString("polar motor to %1").arg(pre_polar_angle));
					// measure
					// save one line.
				}
			}
			// save an file, save pattern picture.
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
	delete spectrum->chart()->series()[0];  // delete old series.
	spectrum->chart()->addSeries(spectrum_data);
}

void Mission::new_pattern() {
	pattern_data->clear();
}

void Mission::draw_pattern_add_point(double angle, double max_power) {
	// add measure point to pattern data
	pattern_data->append(angle, max_power);
}
