/*----------------------------------------------------------------------*
 * Arduino Button Library v1.0                                          *
 * Jack Christensen May 2011, published Mar 2012                        *
 *                                                                      *
 * Library for reading momentary contact switches like tactile button   *
 * switches. Intended for use in state machine constructs.              *
 * Use the read() function to read all buttons in the main loop,        *
 * which should execute as fast as possible.                            *
 *                                                                      *
 * This work is licensed under the Creative Commons Attribution-        *
 * ShareAlike 3.0 Unported License. To view a copy of this license,     *
 * visit http://creativecommons.org/licenses/by-sa/3.0/ or send a       *
 * letter to Creative Commons, 171 Second Street, Suite 300,            *
 * San Francisco, California, 94105, USA.                               *
 *----------------------------------------------------------------------*/

#include "BtnMirror.h"

BtnMirror::BtnMirror(uint8_t pin, uint8_t invert, uint32_t dbTime)
{
    _pin = pin;
    _invert = false; //invert;
    _dbTime = dbTime;
    virtualPin = _invert;
    _state = virtualPin;
    //if (_invert != 0) _state = !_state;
    _time = millis();
    _lastState = _state;
    _changed = 0;
    _lastTime = _time;
    _lastChange = _time;
}

uint8_t BtnMirror::read(void)
{
    static uint32_t ms;
    static uint8_t pinVal;

    ms = millis();

#ifdef HAVE_BUTTON
    if ((!virtualPin) && btn)
        virtualPin = btn->isPressed();
#endif
    pinVal = virtualPin;
    //virtualPin = 0;

    //if (_invert != 0) pinVal = !pinVal;
    if (ms - _lastChange < _dbTime) {
        _lastTime = _time;
        _time = ms;
        _changed = 0;
        return _state;
    }
    else {
        _lastTime = _time;
        _lastState = _state;
        _state = pinVal;
        _time = ms;
        if (_state != _lastState)   {
            _lastChange = ms;
            _changed = 1;
        }
        else {
            _changed = 0;
        }
        virtualPin = 0;
        return _state;
    }
}

uint8_t BtnMirror::readAxis()
{
    static uint32_t ms;
    static uint8_t pinVal;
    static uint16_t val;

    ms = millis();

#ifdef HAVE_BUTTON
    if ((!virtualPin) && btn)
        virtualPin = btn->isAxisPressed();
#endif
    val = virtualPin * 1980;
    //virtualPin = 0;

    if (val > 3900) {
        pinVal = 1;
        _axis = DPAD_V_FULL;
    } else if (val > 1500 && val < 2000) {
        pinVal = 1;
        _axis = DPAD_V_HALF;
    } else {
        pinVal = 0;
        _axis = DPAD_V_NONE;
    }

    //if (_invert == 0) pinVal = !pinVal;
    if (ms - _lastChange < _dbTime) {
        _lastTime = _time;
        _time = ms;
        _changed = 0;
        return _state;
    }
    else {
        _lastTime = _time;
        _lastState = _state;
        _state = pinVal;
        _time = ms;
        if (_state != _lastState)   {
            _lastChange = ms;
            _changed = 1;
        }
        else {
            _changed = 0;
        }
        virtualPin = 0;
        return _state;
    }
    return _state && _changed;
}


/*----------------------------------------------------------------------*
 * isPressed() and isReleased() check the button state when it was last *
 * read, and return false (0) or true (!=0) accordingly.                *
 * These functions do not cause the button to be read.                  *
 *----------------------------------------------------------------------*/
uint8_t BtnMirror::isPressed(void)
{
    return _state == 0 ? 0 : 1;
}

uint8_t BtnMirror::isAxisPressed(void)
{
    if (_state)
        return _axis;
    else
        return 0;
}

uint8_t BtnMirror::isReleased(void)
{
    return _state == 0 ? 1 : 0;
}

/*----------------------------------------------------------------------*
 * wasPressed() and wasReleased() check the button state to see if it   *
 * changed between the last two reads and return false (0) or           *
 * true (!=0) accordingly.                                              *
 * These functions do not cause the button to be read.                  *
 *----------------------------------------------------------------------*/
uint8_t BtnMirror::wasPressed(void)
{
    return _state && _changed;
}

uint8_t BtnMirror::wasAxisPressed(void)
{
    if (_state && _changed)
        return _axis;
    else
        return 0;
}

uint8_t BtnMirror::wasReleased(void)
{
    return !_state && _changed;
}
/*----------------------------------------------------------------------*
 * pressedFor(ms) and releasedFor(ms) check to see if the button is     *
 * pressed (or released), and has been in that state for the specified  *
 * time in milliseconds. Returns false (0) or true (1) accordingly.     *
 * These functions do not cause the button to be read.                  *
 *----------------------------------------------------------------------*/
uint8_t BtnMirror::pressedFor(uint32_t ms)
{
    return (_state == 1 && _time - _lastChange >= ms) ? 1 : 0;
}

uint8_t BtnMirror::releasedFor(uint32_t ms)
{
    return (_state == 0 && _time - _lastChange >= ms) ? 1 : 0;
}
/*----------------------------------------------------------------------*
 * lastChange() returns the time the button last changed state,         *
 * in milliseconds.                                                     *
 *----------------------------------------------------------------------*/
uint32_t BtnMirror::lastChange(void)
{
    return _lastChange;
}
