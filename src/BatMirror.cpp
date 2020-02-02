
#include "BatMirror.h"

BatMirror::BatMirror(Battery *bat) {
    _bat = bat;
}

void BatMirror::begin() {
    //_bat->begin();
}

double BatMirror::getVoltage() {
#ifdef HAVE_BATTERY
    return _bat->getVoltage();
#else
    return 0;
#endif
}

int BatMirror::getPercentage() {
#ifdef HAVE_BATTERY
    return _bat->getPercentage();
#else
    return 99;
#endif
}

void BatMirror::setProtection(bool enable) {
#ifdef HAVE_BATTERY
    _bat->setProtection(enable);
#endif
}

void BatMirror::update() {
    //_bat->update();
}
