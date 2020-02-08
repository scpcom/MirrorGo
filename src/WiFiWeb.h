#ifndef _WiFiWeb_H_
#define _WiFiWeb_H_

#include <Arduino.h>
#include <bm_alloc.h>

extern bool sd_init();

extern bool setupWiFiWeb(String directory);
extern bool showWiFiWeb();

extern void SetWifiWebAppData(String directory);

extern void handleWiFiWeb();

extern bool LoadWifiWebMode();
extern bool SaveWifiWebMode();

#endif
