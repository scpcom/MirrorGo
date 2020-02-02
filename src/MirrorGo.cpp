
#include "MirrorGo.h"

void CGoMirror::begin(unsigned long baud) {
#ifdef HAVE_GO
    GO.begin(baud);

#else
    // UART
    Serial.begin(baud);

    Serial.flush();
    Serial.print("MirrorGo initializing...");

    SPI.begin();

    Serial.println("OK");
#endif

    Speaker.begin();
    lcd.begin();
    battery.begin();

#ifdef HAVE_GO
    BtnA.btn = &GO.BtnA;
    BtnB.btn = &GO.BtnB;
    BtnMenu.btn = &GO.BtnMenu;
    BtnVolume.btn = &GO.BtnVolume;
    BtnSelect.btn = &GO.BtnSelect;
    BtnStart.btn = &GO.BtnStart;
    JOY_Y.btn = &GO.JOY_Y;
    JOY_X.btn = &GO.JOY_X;
#endif

    led.begin();
}

void CGoMirror::update() {
    handleWiFiWeb();

#ifdef HAVE_GO
    GO.update();
#endif
    
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
