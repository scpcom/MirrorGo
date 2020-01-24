
#include "MirrorGo.h"

void CGoMirror::begin(unsigned long baud) {
    GO.begin(baud);

    Speaker.begin();
    lcd.begin();
    battery.begin();

    BtnA.btn = &GO.BtnA;
    BtnB.btn = &GO.BtnB;
    BtnMenu.btn = &GO.BtnMenu;
    BtnVolume.btn = &GO.BtnVolume;
    BtnSelect.btn = &GO.BtnSelect;
    BtnStart.btn = &GO.BtnStart;
    JOY_Y.btn = &GO.JOY_Y;
    JOY_X.btn = &GO.JOY_X;

    led.begin();
}

void CGoMirror::update() {
/*
    if (wifiweb)
        wifiweb->handleClient();
*/

    GO.update();
    
    //Button update
    BtnA.read();
    BtnB.read();
    BtnMenu.read();
    BtnVolume.read();
    BtnSelect.read();
    BtnStart.read();
    JOY_Y.readAxis();
    JOY_X.readAxis();

    Speaker.update();
    battery.update();
}

CGoMirror MG;
