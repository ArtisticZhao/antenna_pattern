#pragma once
#include <QObject>
#include "N9918a.h"
#include "Rotator.h"
#include "PolarMotor.h"

enum class MissonType {
	RadiationPattern,
	Polarization
};

class Mission : public QObject{
	Q_OBJECT
signals:
	void logging(LOGLEVEL level, QString msg);

public:
	Mission(MissonType type, bool polar_enable, double polar_start, double polar_stop, double polar_step,
		bool rotator_enable, double rotator_start, double rotator_stop, double rotator_step,
		N9918a* n9918a, PolarMotor* polar_motor, Rotator* rotator
	);

	void set_pattern_widgets(QChartView* spectrum, QChartView* pattern) {
		this->spectrum = spectrum;
		this->pattern = pattern;
		init_draw();
	}

	void mission_start(QString filepath, QString filename);

private:
	MissonType type;

	PolarMotor* polar_motor;
	bool polar_enable;
	double polar_start;
	double polar_stop;
	double polar_step;

	Rotator* rotator;
	bool rotator_enable;
	double rotator_start;
	double rotator_stop;
	double rotator_step;

	N9918a* n9918a;

	QChartView* spectrum;
	QLineSeries* spectrum_data;
	QChartView* pattern;
	QSplineSeries* pattern_data;
	
	bool is_all_ready() {
		return (n9918a->deviceOK == CONNECTED) && rotator->comOk && polar_motor->comOk;
	}
	void init_draw();
	void draw_spectrum(QLineSeries* lineseries);
	void new_pattern();
	void draw_pattern_add_point(double angle, double max_power);
};

