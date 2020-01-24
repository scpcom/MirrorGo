#include "LedMirror.h"

LedMirror::LedMirror(uint8_t pin)
{
    _pin = pin;
}

void LedMirror::begin(void)
{
    pinMode(_pin, OUTPUT);
    set(false);
}

bool LedMirror::get(void)
{
    return _isOn;
}

bool LedMirror::isOn(void)
{
    return _isOn;
}

void LedMirror::set(bool enable)
{
    _isOn = enable;

    if (_isOn)
        digitalWrite(_pin, HIGH);
    else
        digitalWrite(_pin, LOW);
}

void LedMirror::on(void)
{
    set(true);
}

void LedMirror::off(void)
{
    set(false);
}
