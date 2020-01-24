
#include "BatMirror.h"

BatMirror::BatMirror(Battery *bat) {
    _bat = bat;
}

void BatMirror::begin() {
    //_bat->begin();
}

double BatMirror::getVoltage() {
    return _bat->getVoltage();
}

int BatMirror::getPercentage() {
    return _bat->getPercentage();
}

void BatMirror::setProtection(bool enable) {
    _bat->setProtection(enable);
}

void BatMirror::update() {
    //_bat->update();
}
