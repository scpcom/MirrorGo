#ifndef _SpkMirror_H_
#define _SpkMirror_H_

#include <utility/Speaker.h>

class SpkMirror {
public:
    SpkMirror(SPEAKER *spk);
    
    void begin();
    void end();
    void mute();
    void tone(uint16_t frequency);
    void tone(uint16_t frequency, uint32_t duration);
    void beep();
    void setBeep(uint16_t frequency, uint16_t duration);
    void update();
    
    void write(uint8_t value);
    void setVolume(uint8_t volume);
    void playMusic(const uint8_t* music_data, uint16_t sample_rate);

private:
    SPEAKER *_spk;
};

#endif