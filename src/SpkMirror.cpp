
#include "SpkMirror.h"

SpkMirror::SpkMirror(SPEAKER *spk) {
    _spk = spk;
}

void SpkMirror::begin() {
    //_spk->begin();
}

void SpkMirror::end() {
#ifdef HAVE_SPEAKER
    _spk->end();
#endif
}

void SpkMirror::tone(uint16_t frequency) {
#ifdef HAVE_SPEAKER
    _spk->tone(frequency);
#endif
}

void SpkMirror::tone(uint16_t frequency, uint32_t duration) {
#ifdef HAVE_SPEAKER
    _spk->tone(frequency, duration);
#endif
}

void SpkMirror::beep() {
#ifdef HAVE_SPEAKER
    _spk->beep();
#endif
}

void SpkMirror::setBeep(uint16_t frequency, uint16_t duration) {
#ifdef HAVE_SPEAKER
    _spk->setBeep(frequency, duration);
#endif
}

void SpkMirror::setVolume(uint8_t volume) {
#ifdef HAVE_SPEAKER
    _spk->setVolume(volume);
#endif
}

void SpkMirror::mute() {
#ifdef HAVE_SPEAKER
    _spk->mute();
#endif
}

void SpkMirror::update() {
    //_spk->update();
}

void SpkMirror::write(uint8_t value) {
#ifdef HAVE_SPEAKER
    _spk->write(value);
#endif
}

void SpkMirror::playMusic(const uint8_t* music_data, uint16_t sample_rate) {
#ifdef HAVE_SPEAKER
    _spk->playMusic(music_data, sample_rate);
#endif
}

