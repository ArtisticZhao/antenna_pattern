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
	void logging(LogLevel level, QString msg);
	void status_changed(bool busy);
	void process_changed(int process_val);

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

	void mission_start(QString file_full);
	void mission_stop();

private:
	bool stop_signal;
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
	double power_max, power_min; // save max and min to set axis range.

	QString filepath;
	QList<QString> save_data;
	QFile* file_obj;

	// ��������ر���
	int mission_num;
	int mission_count;
	
	void measure_single_loop(std::vector<double>* innerloop, MotorCtrl* inner_motor, QString save_filename);

	bool is_all_ready() {
		return (n9918a->deviceOK == DevStatus::connected) && rotator->comOk && polar_motor->comOk;
	}
	void init_draw();
	void draw_spectrum(QLineSeries* lineseries);
	void new_pattern();
	void draw_pattern_add_point(double angle, double max_power);
	// file
	bool create_folder(QString file_full);
	bool create_result_file(QString filename);
	void append_result(double current_angle, QString n9918_result);
	bool save_result_to_file();
	void save_pattern_to_file(QString filename);
};

