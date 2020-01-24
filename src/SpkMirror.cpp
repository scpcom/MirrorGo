
#include "SpkMirror.h"

SpkMirror::SpkMirror(SPEAKER *spk) {
    _spk = spk;
}

void SpkMirror::begin() {
    //_spk->begin();
}

void SpkMirror::end() {
    _spk->end();
}

void SpkMirror::tone(uint16_t frequency) {
    _spk->tone(frequency);
}

void SpkMirror::tone(uint16_t frequency, uint32_t duration) {
    _spk->tone(frequency, duration);
}

void SpkMirror::beep() {
    _spk->beep();
}

void SpkMirror::setBeep(uint16_t frequency, uint16_t duration) {
    _spk->setBeep(frequency, duration);
}

void SpkMirror::setVolume(uint8_t volume) {
    _spk->setVolume(volume);
}

void SpkMirror::mute() {
    _spk->mute();
}

void SpkMirror::update() {
    //_spk->update();
}

void SpkMirror::write(uint8_t value) {
    _spk->write(value);
}

void SpkMirror::playMusic(const uint8_t* music_data, uint16_t sample_rate) {
    _spk->playMusic(music_data, sample_rate);
}

