# MirrorGo
A library to mirror Go on WiFi Web Interface


What it does:

- WiFi management
- Web Remote Console (see what is on device screen an press buttons via web interface)
- Integrated WebFileBrowser (derived from https://github.com/ripper121/odroidgowebfilebrowser/)
- Powered by ESPAsyncWebServer, your application and Web Interface can be used at the same time 

Prepare:

1. Setup ODROID-GO on Arduino IDE (https://wiki.odroid.com/odroid_go/arduino/01_arduino_setup)

2. Install AsyncTCP, ESPAsyncTCP and ESPAsyncWebServer
https://github.com/me-no-dev/ESPAsyncWebServer

3. Install MirrorGo
```
cd ~/Arduino/libraries
git clone https://github.com/scpcom/MirrorGo.git
```
How to use it:

1. Add the header on top of your project:
```
#include <MirrorGo.h>
```
2. Replaces all references to GO with MG, examples:

- Replace GO.begin() with MG.begin()
- Replace GO.update() with MG.update()
- Replace GO.BtnA.isPressed() with MG.BtnA.isPressed()

3. Replace SD.begin() with sd_init(), setup() should now look like this:
```
void setup(void) {
    MG.begin(115200);

    sd_init();

    setupWiFiWeb("/odroid/data/myapp");

    // ......
}
```

4. If your application does have a menu, add an menu entry where you call this:
```
showWiFiWeb();
```
(if you not have a menu or something else just add this to the setup() section)

*NOTE*: This will add 640 kB to your binary which will make your application bigger than 1MB. if you use mkfw make sure you choose a bigger partition size.
