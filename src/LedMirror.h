#ifndef _LEDMIRROR_H_
#define _LEDMIRROR_H_

#include <Arduino.h>

#ifndef PIN_BLUE_LED
#define PIN_BLUE_LED 2
#endif

class LedMirror {
public:
	LedMirror(uint8_t pin);

	void begin(void);
	bool get(void);
	bool isOn(void);
	void set(bool enable);
	void on(void);
	void off(void);

private:
	uint8_t _pin;
	bool _isOn;
};

#endif /* _LEDMIRROR_H_ */
