#ifndef _BATMIRROR_H_
#define _BATMIRROR_H_

#include <Arduino.h>
#include <utility/battery.h>

class BatMirror {
public:
	BatMirror(Battery *bat);

	void begin();
	double getVoltage();
	int getPercentage();
	void setProtection(bool enable);
	void update();

private:
	Battery *_bat;
};

#endif /* _BATMIRROR_H_ */
