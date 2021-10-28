#pragma once
// ÐéÄâÀà
class MotorCtrl {
public:
	virtual void turn_to_zero()=0;
	virtual bool turn_to(double angle)=0;
};

