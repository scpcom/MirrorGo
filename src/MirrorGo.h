#ifndef MirrorGo_h
#define MirrorGo_h

#include <odroid_go.h>
#include <LCDMirror.h>
#include <SpkMirror.h>
#include <BtnMirror.h>
#include <BatMirror.h>
#include <LedMirror.h>
#include <WiFiWeb.h>

class CGoMirror {

 public:
    void begin(unsigned long baud = 115200);
    void update();

    #define DEBOUNCE_MS 5
    BtnMirror JOY_Y = BtnMirror(BUTTON_JOY_Y, true, DEBOUNCE_MS);
    BtnMirror JOY_X = BtnMirror(BUTTON_JOY_X, true, DEBOUNCE_MS);
    BtnMirror BtnA = BtnMirror(BUTTON_A_PIN, true, DEBOUNCE_MS);
    BtnMirror BtnB = BtnMirror(BUTTON_B_PIN, true, DEBOUNCE_MS);
    BtnMirror BtnMenu = BtnMirror(BUTTON_MENU, true, DEBOUNCE_MS);
    BtnMirror BtnVolume = BtnMirror(BUTTON_VOLUME, true, DEBOUNCE_MS);
    BtnMirror BtnSelect = BtnMirror(BUTTON_SELECT, true, DEBOUNCE_MS);
    BtnMirror BtnStart = BtnMirror(BUTTON_START, true, DEBOUNCE_MS);

    // LCD
    LCDMirror lcd = LCDMirror(&GO.lcd);
    SpkMirror Speaker = SpkMirror(&GO.Speaker);
    BatMirror battery = BatMirror(&GO.battery);
    LedMirror led = LedMirror(PIN_BLUE_LED);
};

extern CGoMirror MG;

#endif
