
#include <WiFiWeb.h>
#include <MirrorGo.h>

#ifndef ESP32
#define GO MG
#endif

#ifdef ESP32
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <WiFiAP.h>
#include <AsyncTCP.h>
#include <esp_task_wdt.h>
#elif defined(ESP8266)
#include <SD.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFiAP.h>
#include <ESPAsyncTCP.h>
#define SD_CS_PIN SS
#endif
#include <ESPAsyncWebServer.h>
#include <WebAuthentication.h>
#include <DNSServer.h>


#ifdef ESP8266
#define WIFI_AUTH_OPEN		ENC_TYPE_NONE
#define WIFI_AUTH_WEP		ENC_TYPE_WEP
#define WIFI_AUTH_WPA_PSK	ENC_TYPE_TKIP
#define WIFI_AUTH_WPA2_PSK	ENC_TYPE_CCMP
#define WIFI_AUTH_WPA_WPA2_PSK	ENC_TYPE_AUTO
#endif


typedef struct {
  int64_t Size;
  int64_t Free;
  int64_t MinFree;
  int64_t MaxAlloc;
} MemData;

typedef struct {
  MemData SDcard;
  MemData Flash;
  MemData Psram;
  MemData Heap;
} MemInfo;

typedef struct {
  String SSID;
  String PSK;
} WifiData;



const byte DNS_PORT = 53;
const char* host = "odroidgo";
const char* ssid = "odroidgo";
const char* password = "odroidgo";

String ntpServer1 = "pool.ntp.org";
String ntpServer2 = "";
String ntpServer3 = "";
long  gmtOffset_sec = 3600;
int   daylightOffset_sec = 3600;
time_t browserTime = 0;

String WifiAPHost = host;
String WifiAPSSID = ssid;
String WifiAPPSK = password;

IPAddress WifiAPIP = IPAddress(192, 168, 4, 1);
IPAddress WifiAPGW = WifiAPIP;
IPAddress WifiAPMask = IPAddress(255, 255, 255, 0);

String WifiHost = host;
String WifiSSID = "";
String WifiPSK = "";

uint8_t WifiIPMode = 2;

IPAddress WifiIP = IPAddress(0, 0, 0, 0);
IPAddress WifiGW = WifiAPIP;
IPAddress WifiMask = IPAddress(255, 255, 255, 0);
IPAddress WifiDNS1 = IPAddress(0, 0, 0, 0);
IPAddress WifiDNS2 = IPAddress(0, 0, 0, 0);

int WifiCount = 0;
WifiData *WifiList;

String WifiWebUser = ssid;
String WifiWebPass = password;

String WifiWebAppData = "/odroid/data";

uint8_t WifiWebMode = 0;
bool webfileReadonly = true;
bool webfileDisabled = false;
bool webrcAutoRefresh = false;

String NewWifiAPHost = "";
String NewWifiAPSSID = "";
String NewWifiAPPSK = "";

IPAddress NewWifiAPIP = IPAddress(0, 0, 0, 0);
IPAddress NewWifiAPGW = NewWifiAPIP;
IPAddress NewWifiAPMask = IPAddress(255, 255, 255, 0);

String NewWifiHost = "";
String NewWifiSSID = "";
String NewWifiPSK = "";

uint8_t NewWifiIPMode = 0xFF;

IPAddress NewWifiIP = IPAddress(0, 0, 0, 0);
IPAddress NewWifiGW = NewWifiIP;
IPAddress NewWifiMask = IPAddress(255, 255, 255, 0);
IPAddress NewWifiDNS1 = IPAddress(0, 0, 0, 0);
IPAddress NewWifiDNS2 = IPAddress(0, 0, 0, 0);

uint8_t NewWifiWebMode = 0xFF;
uint8_t NewIPMode = NewWifiWebMode;

MemInfo memboot;


const char* header = "<!DOCTYPE html>\n"
                     "<html>\n"
                     "<head>\n"
                     "<meta charset=\"utf-8\">\n"
                     "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                     "<style>\n"
                     "  h1 {"
                     "    border-bottom: 1px solid #c0c0c0;"
                     "    margin-bottom: 10px;"
                     "    padding-bottom: 10px;"
                     "    white-space: nowrap;"
                     "  }"
                     "  table {"
                     "    border-collapse: collapse;"
                     "  }"
                     "  th {"
                     "    cursor: pointer;"
                     "  }"
                     "  td.dCol {"
                     "    -webkit-padding-start: 2em;"
                     "    text-align: end;"
                     "    white-space: nowrap;"
                     "  }"
                     "  td.ledCol {"
                     "    -webkit-padding-start: 2em;"
                     "    text-align: center;"
                     "    white-space: nowrap;"
                     "  }"
                     "  a.icon {"
                     "    -webkit-padding-start: 1.5em;"
                     "    text-decoration: none;"
                     "  }"
                     "  a.icon:hover {"
                     "    text-decoration: underline;"
                     "  }"
                     "  a.file {"
                     "    background : url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAIAAACQkWg2AAAABnRSTlMAAAAAAABupgeRAAABHUlEQVR42o2RMW7DIBiF3498iHRJD5JKHurL+CRVBp+i2T16tTynF2gO0KSb5ZrBBl4HHDBuK/WXACH4eO9/CAAAbdvijzLGNE1TVZXfZuHg6XCAQESAZXbOKaXO57eiKG6ft9PrKQIkCQqFoIiQFBGlFIB5nvM8t9aOX2Nd18oDzjnPgCDpn/BH4zh2XZdlWVmWiUK4IgCBoFMUz9eP6zRN75cLgEQhcmTQIbl72O0f9865qLAAsURAAgKBJKEtgLXWvyjLuFsThCSstb8rBCaAQhDYWgIZ7myM+TUBjDHrHlZcbMYYk34cN0YSLcgS+wL0fe9TXDMbY33fR2AYBvyQ8L0Gk8MwREBrTfKe4TpTzwhArXWi8HI84h/1DfwI5mhxJamFAAAAAElFTkSuQmCC \") left top no-repeat;"
                     "  }"
                     "  a.dir {"
                     "    background : url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAd5JREFUeNqMU79rFUEQ/vbuodFEEkzAImBpkUabFP4ldpaJhZXYm/RiZWsv/hkWFglBUyTIgyAIIfgIRjHv3r39MePM7N3LcbxAFvZ2b2bn22/mm3XMjF+HL3YW7q28YSIw8mBKoBihhhgCsoORot9d3/ywg3YowMXwNde/PzGnk2vn6PitrT+/PGeNaecg4+qNY3D43vy16A5wDDd4Aqg/ngmrjl/GoN0U5V1QquHQG3q+TPDVhVwyBffcmQGJmSVfyZk7R3SngI4JKfwDJ2+05zIg8gbiereTZRHhJ5KCMOwDFLjhoBTn2g0ghagfKeIYJDPFyibJVBtTREwq60SpYvh5++PpwatHsxSm9QRLSQpEVSd7/TYJUb49TX7gztpjjEffnoVw66+Ytovs14Yp7HaKmUXeX9rKUoMoLNW3srqI5fWn8JejrVkK0QcrkFLOgS39yoKUQe292WJ1guUHG8K2o8K00oO1BTvXoW4yasclUTgZYJY9aFNfAThX5CZRmczAV52oAPoupHhWRIUUAOoyUIlYVaAa/VbLbyiZUiyFbjQFNwiZQSGl4IDy9sO5Wrty0QLKhdZPxmgGcDo8ejn+c/6eiK9poz15Kw7Dr/vN/z6W7q++091/AQYA5mZ8GYJ9K0AAAAAASUVORK5CYII= \") left top no-repeat;"
                     "  }"
                     "  a.up {"
                     "    background : url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAmlJREFUeNpsU0toU0EUPfPysx/tTxuDH9SCWhUDooIbd7oRUUTMouqi2iIoCO6lceHWhegy4EJFinWjrlQUpVm0IIoFpVDEIthm0dpikpf3ZuZ6Z94nrXhhMjM3c8895977BBHB2PznK8WPtDgyWH5q77cPH8PpdXuhpQT4ifR9u5sfJb1bmw6VivahATDrxcRZ2njfoaMv+2j7mLDn93MPiNRMvGbL18L9IpF8h9/TN+EYkMffSiOXJ5+hkD+PdqcLpICWHOHc2CC+LEyA/K+cKQMnlQHJX8wqYG3MAJy88Wa4OLDvEqAEOpJd0LxHIMdHBziowSwVlF8D6QaicK01krw/JynwcKoEwZczewroTvZirlKJs5CqQ5CG8pb57FnJUA0LYCXMX5fibd+p8LWDDemcPZbzQyjvH+Ki1TlIciElA7ghwLKV4kRZstt2sANWRjYTAGzuP2hXZFpJ/GsxgGJ0ox1aoFWsDXyyxqCs26+ydmagFN/rRjymJ1898bzGzmQE0HCZpmk5A0RFIv8Pn0WYPsiu6t/Rsj6PauVTwffTSzGAGZhUG2F06hEc9ibS7OPMNp6ErYFlKavo7MkhmTqCxZ/jwzGA9Hx82H2BZSw1NTN9Gx8ycHkajU/7M+jInsDC7DiaEmo1bNl1AMr9ASFgqVu9MCTIzoGUimXVAnnaN0PdBBDCCYbEtMk6wkpQwIG0sn0PQIUF4GsTwLSIFKNqF6DVrQq+IWVrQDxAYQC/1SsYOI4pOxKZrfifiUSbDUisif7XlpGIPufXd/uvdvZm760M0no1FZcnrzUdjw7au3vu/BVgAFLXeuTxhTXVAAAAAElFTkSuQmCC \") left top no-repeat;"
                     "  }"
                     "  html[dir=rtl] a {"
                     "    background-position-x: right;"
                     "  }"
                     "  #parentDirLinkBox {"
                     "    margin-bottom: 10px;"
                     "    padding-bottom: 10px;"
                     "  }"
                     "  #listingParsingErrorBox {"
                     "    border: 1px solid black;"
                     "    background: #fae691;"
                     "    padding: 10px;"
                     "    display: none;"
                     "  }"
                     "\n"
                     ".ledbtn {\n"
                     "  padding: 4px;\n"
                     "  text-align: center;\n"
                     "  text-decoration: none;\n"
                     "  display: inline-block;\n"
                     "  font-size: 10px;\n"
                     "  margin: 2px 1px;\n"
                     "  border-radius: 50%;\n"
                     "}\n"
                     "\n"
                     ".ledbtnoff {\n"
                     "  background-color: white;\n"
                     "  color: #6080D0;\n"
                     "}\n"
                     "\n"
                     ".ledbtnon {\n"
                     "  border: none;\n"
                     "  background-color: #6080D0;\n"
                     "  color: white;\n"
                     "}\n"
                     "</style>\n"
                     "<title id=\"title\">Web FileBrowser</title>\n"
                     "</head>\n"
                     "<body>\n";

const char* footer =  "<hr>\n"
                      "<br>WiFi Web - Based on:\n"
                      "<br>WebFileBrowser - Made with &hearts; and &#9749; by ripper121<br>\n"
                      "<hr>\n"
                      "</body>\n"
                      "</html>\n";

const char* script = "<script>\n"
                      "function sortTable(l){var e=document.getElementById(\"theader\"),n=e.cells[l].dataset.order||\"1\",s=0-(n=parseInt(n,10));e.cells[l].dataset.order=s;var t,a=document.getElementById(\"tbody\"),r=a.rows,d=[];for(t=0;t<r.length;t++)d.push(r[t]);for(d.sort(function(e,t){var a=e.cells[l].dataset.value,r=t.cells[l].dataset.value;return l?(a=parseInt(a,10),(r=parseInt(r,10))<a?s:a<r?n:0):r<a?s:a<r?n:0}),t=0;t<d.length;t++)a.appendChild(d[t])}\n"
                      "\n"
                      "</script>\n";

const char* footer_script = "<script>\n"
                      "function updateImages()\n"
                      "{\n"
                      "      var allimages= document.getElementsByTagName('img');\n"
                      "      for (var i=0; i<allimages.length; i++) {\n"
                      "          if (allimages[i].getAttribute('data-src')) {\n"
                      "              allimages[i].setAttribute('src', allimages[i].getAttribute('data-src') + \"?time=\" + new Date().getTime());\n"
                      "          }\n"
                      "      }\n"
                      "}\n"
                      "function updateImage2()\n"
                      "{\n"
                      "    updateImages();\n"
                      "    setTimeout(updateImages, 100);\n"
                      "}\n"
                      "function updateImage3()\n"
                      "{\n"
                      "    updateImages();\n"
                      "    setTimeout(updateImage2, 200);\n"
                      "}\n"
                      "function updateLed()\n"
                      "{\n"
                      "    var led = document.getElementById('press-LED');\n"
                      "    if (led.getAttribute('class') == \"ledbtn ledbtnon\") {\n"
                      "        led.setAttribute('class', \"ledbtn ledbtnoff\");\n"
                      "    } else {\n"
                      "        led.setAttribute('class', \"ledbtn ledbtnon\");\n"
                      "    }\n"
                      "}\n"
                      "function press(button) {\n"
                      "    var host = location.protocol+\"//\"+location.hostname;\n"
                      "    var checkurl = host+\"/button?press=\"+button;\n"
                      "    var http = new XMLHttpRequest();\n"
                      "\n"
                      "    http.onreadystatechange = function() {\n"
                      "        if (http.readyState == 4 && http.status == 200) {\n"

                      "            if (button == \"LED\") {\n"
                      "                updateLed();\n"
                      "            } else {\n"
                      "                setTimeout(updateImage3, 200);\n"
                      "            }\n"
                      "        }\n"
                      "    };\n"
                      "\n"
                      "    http.open(\"GET\", checkurl, true);\n"
                      "    http.send();\n"
                      "}\n"
                      "</script>\n"                      
                      "\n"
                      "<script type=\"module\">\n"
                      "window.addEventListener('load', function(){\n"
                      "    setTimeout(updateImages, 100);\n"
                      "}, false)\n"
                      "</script>\n"                      
                      "\n"
                      "<script nomodule>\n"
                      "window.addEventListener('load', function(){\n"
                      "    setTimeout(updateImages, 100);\n"
                      "}, false)\n"
                      "</script>\n";

const char* goback_script = "<script>\n"
                      "function goBack()\n"
                      "{\n"
                      "    location.href='/';\n"
                      "}\n"
                      "</script>\n"                      
                      "\n"
                      "<script type=\"module\">\n"
                      "window.addEventListener('load', function(){\n"
                      "    setTimeout(goBack, 100);\n"
                      "}, false)\n"
                      "</script>\n"                      
                      "\n"
                      "<script nomodule>\n"
                      "window.addEventListener('load', function(){\n"
                      "    setTimeout(goBack, 100);\n"
                      "}, false)\n"
                      "</script>\n";

const char* setwifi_script = ""
                      "function updateWifi(m)\n"
                      "{\n"
                      "    var essid = document.getElementById('wifissid');\n"
                      "    var epsk  = document.getElementById('wifipsk');\n"
                      "    var estat = document.getElementById('ipstatic');\n"
                      "    var edhcp = document.getElementById('ipdhcp');\n"
                      "    var eip   = document.getElementById('wifiip');\n"
                      "    var emask = document.getElementById('wifimask');\n"
                      "    var egw   = document.getElementById('wifigw');\n"
                      "    var edns1 = document.getElementById('wifidns1');\n"
                      "    var edns2 = document.getElementById('wifidns2');\n"
                      "    var ehost = document.getElementById('wifihost');\n"
                      "    var ipmode = 0;\n"
                      "    if (estat.checked) {\n"
                      "      ipmode = 1;\n"
                      "    } else if (edhcp.checked) {\n"
                      "      ipmode = 2;\n"
                      "    }\n"
                      "    if (WifiWebMode == 1) {\n"
                      "        WifiAPSSID = essid.value;\n"
                      "        WifiAPPSK  = epsk.value;\n"
                      "        WifiAPIP   = eip.value;\n"
                      "        WifiAPMask = emask.value;\n"
                      "        WifiAPGW   = egw.value;\n"
                      "        WifiAPHost = ehost.value;\n"
                      "    } else if (WifiWebMode == 2) {\n"
                      "        WifiSSID   = essid.value;\n"
                      "        WifiPSK    = epsk.value;\n"
                      "        WifiIP     = eip.value;\n"
                      "        WifiMask   = emask.value;\n"
                      "        WifiGW     = egw.value;\n"
                      "        WifiDNS1   = edns1.value;\n"
                      "        WifiDNS2   = edns2.value;\n"
                      "        WifiHost   = ehost.value;\n"
                      "        WifiIPMode = ipmode;\n"
                      "    }\n"
                      "    ipmode = 0;\n"
                      "    if (m == 1) {\n"
                      "        essid.value = WifiAPSSID;\n"
                      "        epsk.value  = WifiAPPSK;\n"
                      "        eip.value   = WifiAPIP;\n"
                      "        emask.value = WifiAPMask;\n"
                      "        egw.value   = WifiAPGW;\n"
                      "        edns1.value = WifiAPIP;\n"
                      "        edns2.value = \"\";\n"
                      "        ehost.value = WifiAPHost;\n"
                      "        ipmode = 1;\n"
                      "    } else if (m == 2) {\n"
                      "        essid.value = WifiSSID;\n"
                      "        epsk.value  = WifiPSK;\n"
                      "        eip.value   = WifiIP;\n"
                      "        emask.value = WifiMask;\n"
                      "        egw.value   = WifiGW;\n"
                      "        edns1.value = WifiDNS1;\n"
                      "        edns2.value = WifiDNS2;\n"
                      "        ehost.value = WifiHost;\n"
                      "        ipmode = WifiIPMode;\n"
                      "    } else {\n"
                      "        essid.value = \"\"\n"
                      "        epsk.value  = \"\";\n"
                      "        eip.value   = \"\";\n"
                      "        emask.value = \"\";\n"
                      "        egw.value   = \"\";\n"
                      "        edns1.value = \"\";\n"
                      "        edns2.value = \"\";\n"
                      "        ehost.value = \"\";\n"
                      "    }\n"
                      "    WifiWebMode = m;\n"
                      "    essid.disabled = (WifiWebMode == 0);\n"
                      "    epsk.disabled  = (WifiWebMode == 0);\n"
                      "    estat.disabled = (WifiWebMode == 0);\n"
                      "    edhcp.disabled = (WifiWebMode == 0);\n"
                      "    ehost.disabled = (WifiWebMode == 0);\n"
                      "    estat.checked = (ipmode == 1);\n"
                      "    edhcp.checked = (ipmode == 2);\n"
                      "    updateIP(ipmode);\n"
                      "}\n"
                      "function updateIP(m)\n"
                      "{\n"
                      "    var eip   = document.getElementById('wifiip');\n"
                      "    var emask = document.getElementById('wifimask');\n"
                      "    var egw   = document.getElementById('wifigw');\n"
                      "    var edns1 = document.getElementById('wifidns1');\n"
                      "    var edns2 = document.getElementById('wifidns2');\n"
                      "    eip.disabled   = (m != 1);\n"
                      "    emask.disabled = (m != 1);\n"
                      "    egw.disabled   = (m != 1);\n"
                      "    edns1.disabled = (m != 1);\n"
                      "    edns2.disabled = (m != 1);\n"
                      "}\n"
                      "function updateSSID(l) {\n"
                      "  var t,a=document.getElementById(\"wntbody\"),r=a.rows;\n"
                      "  var e=document.getElementById(\"wifissid\");\n"
                      "  e.value=r[l].cells[0].innerHTML;\n"
                      "}\n";

const char* settings_script = ""
                      "function add_time(link)\n"
                      "{\n"
                      "  location.href=link.href + '&time=' + new Date().getTime();\n"
                      "  return false;\n"
                      "}\n"
                      "function updateTime() {\n"
                      "  var ti = document.getElementById('time');\n"
                      "  ti.value = new Date().getTime();\n"
                      "}\n"
                      "function updateQuorum() {\n"
                      "  var ntps1 = document.getElementById('ntpserver1');\n"
                      "  var ntps2 = document.getElementById('ntpserver2');\n"
                      "  var ntps3 = document.getElementById('ntpserver3');\n"
                      "  if ((ntps2.value == \"\") && (ntps3.value == \"\")) {\n"
                      "    ntps3.value = '2.'+ntps1.value;\n"
                      "    ntps2.value = '1.'+ntps1.value;\n"
                      "    ntps1.value = '0.'+ntps1.value;\n"
                      "  }\n"
                      "  return false;\n"
                      "}\n";


bool dnsStarted = false;
DNSServer dnsServer;

String uploadPath = "";
AsyncWebServer server(80);

AsyncWebServer *wifiweb = NULL;

AsyncWebServer *wifiweb_init(bool interactive = true);


unsigned char h2int(char c)
{
  if (c >= '0' && c <= '9') {
    return ((unsigned char)c - '0');
  }
  if (c >= 'a' && c <= 'f') {
    return ((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <= 'F') {
    return ((unsigned char)c - 'A' + 10);
  }
  return (0);
}

String urldecode(String str)
{

  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == '+') {
      encodedString += ' ';
    } else if (c == '%') {
      i++;
      code0 = str.charAt(i);
      i++;
      code1 = str.charAt(i);
      c = (h2int(code0) << 4) | h2int(code1);
      encodedString += c;
    } else {

      encodedString += c;
    }

    yield();
  }

  return encodedString;
}

String str_size(int64_t bytes, int div, String unit) {
  String fsize = "";
  if (bytes < div)                    fsize = String((int32_t)bytes) + " "+unit;
  else if (bytes < (div * div))       fsize = String(bytes / float(div), 2) + " K"+unit;
  else if (bytes < (div * div * div)) fsize = String(bytes / float(div) / float(div), 2) + " M"+unit;
  else                                fsize = String(bytes / float(div) / float(div) / float(div), 2) + " G"+unit;
  return fsize;
}

String file_size(int64_t bytes) {
  return str_size(bytes, 1024, "B");
}

String freq_hz(int64_t hz) {
  return str_size(hz, 1000, "Hz");
}

#ifndef ESP32
bool getLocalTime(struct tm * info, uint32_t ms = 5000)
{
    uint32_t start = millis();
    time_t now;
    while((millis()-start) <= ms) {
        time(&now);
        localtime_r(&now, info);
        if(info->tm_year > (2016 - 1900)){
            return true;
        }
        delay(10);
    }
    return false;
}
#endif

String getTimeString(time_t t)
{
  struct tm timeinfo;
  char buffer[26];
  String ret = "";

  if (!t) {
    return ret;
  }

  localtime_r(&t, &timeinfo);

  strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", &timeinfo);
  ret = buffer;
  return ret;
}

String getLocalTimeString()
{
  /* getLocalTime() uses delay() which is not suitable for async TCP */
  time_t now;
  time(&now);
  return getTimeString(now);
}

String getBrowserTimeString()
{
  return getTimeString(browserTime);
}

String getContentType(AsyncWebServerRequest *request, String filename) {
  filename.toUpperCase();
  if (request->hasArg("download")) return "application/octet-stream";

  else if (filename.endsWith(".BIN")) return "application/octet-stream";
  else if (filename.endsWith(".COM")) return "application/octet-stream";
  else if (filename.endsWith(".DAT")) return "application/octet-stream";
  else if (filename.endsWith(".FW")) return "application/octet-stream";
  else if (filename.endsWith(".IMG")) return "application/octet-stream";
  else if (filename.endsWith(".ROM")) return "application/octet-stream";

  else if (filename.endsWith(".SAV")) return "application/octet-stream";
  else if (filename.endsWith(".GB")) return "application/octet-stream";
  else if (filename.endsWith(".GBC")) return "application/octet-stream";
  else if (filename.endsWith(".NES")) return "application/octet-stream";
  else if (filename.endsWith(".WAD")) return "application/octet-stream";
  else if (filename.endsWith(".DSG")) return "application/octet-stream";
  else if (filename.endsWith(".CK1")) return "application/octet-stream";
  else if (filename.endsWith(".CK2")) return "application/octet-stream";
  else if (filename.endsWith(".CK3")) return "application/octet-stream";
  else if (filename.endsWith(".CK4")) return "application/octet-stream";

  else if (filename.endsWith(".DLL")) return "application/vnd.microsoft.portable-executable";
  else if (filename.endsWith(".EXE")) return "application/vnd.microsoft.portable-executable";

  else if (filename.endsWith(".HTM")) return "text/html";
  else if (filename.endsWith(".HTML")) return "text/html";
  else if (filename.endsWith(".CSS")) return "text/css";
  else if (filename.endsWith(".JS")) return "application/javascript";
  else if (filename.endsWith(".JSON")) return "application/json";
  else if (filename.endsWith(".BMP")) return "image/bmp";
  else if (filename.endsWith(".PNG")) return "image/png";
  else if (filename.endsWith(".GIF")) return "image/gif";
  else if (filename.endsWith(".JPG")) return "image/jpeg";
  else if (filename.endsWith(".JPEG")) return "image/jpeg";
  else if (filename.endsWith(".ICO")) return "image/x-icon";
  else if (filename.endsWith(".SVG")) return "image/svg+xml";
  else if (filename.endsWith(".TGA")) return "image/tga";
  else if (filename.endsWith(".EOT")) return "font/eot";
  else if (filename.endsWith(".WOFF")) return "font/woff";
  else if (filename.endsWith(".WOFF2")) return "font/woff2";
  else if (filename.endsWith(".TTF")) return "font/ttf";
  else if (filename.endsWith(".XML")) return "text/xml";
  else if (filename.endsWith(".SHTML")) return "application/xhtml+xml";
  else if (filename.endsWith(".XHTML")) return "application/xhtml+xml";

  else if (filename.endsWith(".XLS")) return "application/vnd.ms-excel";
  else if (filename.endsWith(".XLA")) return "application/vnd.ms-excel";
  else if (filename.endsWith(".XLT")) return "application/vnd.ms-excel";
  else if (filename.endsWith(".PPT")) return "application/vnd.ms-powerpoint";
  else if (filename.endsWith(".PPZ")) return "application/vnd.ms-powerpoint";
  else if (filename.endsWith(".PPS")) return "application/vnd.ms-powerpoint";
  else if (filename.endsWith(".POT")) return "application/vnd.ms-powerpoint";
  else if (filename.endsWith(".DOC")) return "application/msword";
  else if (filename.endsWith(".DOT")) return "application/msword";

  else if (filename.endsWith(".XLSX")) return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
  else if (filename.endsWith(".XLTX")) return "application/vnd.openxmlformats-officedocument.spreadsheetml.template";
  else if (filename.endsWith(".PPTX")) return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
  else if (filename.endsWith(".PPSX")) return "application/vnd.openxmlformats-officedocument.presentationml.slideshow";
  else if (filename.endsWith(".POTX")) return "application/vnd.openxmlformats-officedocument.presentationml.template";
  else if (filename.endsWith(".DOCX")) return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
  else if (filename.endsWith(".DOTX")) return "application/vnd.openxmlformats-officedocument.wordprocessingml.template";

  else if (filename.endsWith(".PDF")) return "application/x-pdf";
  else if (filename.endsWith(".ZIP")) return "application/x-zip";
  else if (filename.endsWith(".GZ")) return "application/x-gzip";
  return "text/plain";
}

void doRequestAuthentication(AsyncWebServerRequest *request, const char * realm = NULL, const String& authFailMsg = String(""),  bool isDigest = true){
  AsyncWebServerResponse * r = request->beginResponse(401, "text/html", authFailMsg);
  if(!isDigest && realm == NULL){
    r->addHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
  } else if(!isDigest){
    String header = "Basic realm=\"";
    header.concat(realm);
    header.concat("\"");
    r->addHeader("WWW-Authenticate", header);
  } else {
    String header = "Digest ";
    header.concat(requestDigestAuthentication(realm));
    r->addHeader("WWW-Authenticate", header);
  }
  request->send(r);
}

bool handleLogin(AsyncWebServerRequest *request) {
  if (request->authenticate(WifiWebUser.c_str(), WifiWebPass.c_str())) {
    return true;
  } else { 
    String webpage = "";
    webpage += F("<html>Authentication failed</html>");
    doRequestAuthentication(request, "Secure", webpage);
  }
  
  return false;
}

void handleNotFound(AsyncWebServerRequest *request) {
  String webpage = "";
  webpage += header;
  webpage += F("<hr>File Not Found<br>");
  webpage += F("<br>URI:");
  webpage += request->url();
  webpage += F("<br>Method: ");
  webpage += (request->method() == HTTP_GET) ? "GET" : "POST";
  webpage += F("<br>Arguments: ");
  webpage += request->args();
  webpage += F("<br>");
  for (uint8_t i = 0; i < request->args(); i++) {
    webpage += request->argName(i) + ": " + request->arg(i) + "<br>";
  }
  webpage += F("<br><button class='buttons' onclick=\"location.href='/';\">OK</button>");
  webpage += footer;
  request->send(404, "text/html", webpage);
}

void doMkdir(AsyncWebServerRequest *request) {
  if (!handleLogin(request))
    return;

  if (webfileReadonly) {
    handleNotFound(request);
    return;
  }

  String webpage = "";
  webpage += header;
  String path = "";
  String dirName = "";

  for (uint8_t i = 0; i < request->args(); i++) {
    if (request->argName(i) == "dirName") {
      Serial.printf("Dir Name: %s\n", request->arg(i).c_str());
      dirName =  request->arg(i);
    }
    if (request->argName(i) == "path") {
      Serial.printf("Path: %s\n", request->arg(i).c_str());
      path = request->arg(i);
    }

  }

  if (dirName != "" && path != "") {
    webpage += F("<hr>Creating Dir: <br>");

    if (path == "/")
      path = path + dirName;
    else
      path = path + "/" + dirName;

    webpage += path;

    Serial.printf("Creating Dir: %s\n", path.c_str());
    if (SD.mkdir(path)) {
      webpage += F("<br>Dir created<br>");
    } else {
      Serial.println("mkdir failed");
      webpage += F("<br>mkdir failed<br>");
    }
  } else {
    webpage += F("<br>Path or Name empty!");
  }

  webpage += F("<br><button class='buttons' onclick=\"location.href='/';\">OK</button>");
  webpage += footer;
  request->send(200, "text/html", webpage);
}

void doDelete(AsyncWebServerRequest *request) {
  if (!handleLogin(request))
    return;

  if (webfileReadonly) {
    handleNotFound(request);
    return;
  }

  String webpage = "";
  webpage += header;
  for (uint8_t i = 0; i < request->args(); i++) {
    if (request->argName(i) == "file") {
      Serial.printf("Deleting file: %s\n", request->arg(i).c_str());
      webpage += F("<hr>Deleting file: <br>");
      webpage += request->arg(i);
      if (SD.remove(request->arg(i))) {
        webpage += F("<br>File deleted<br>");
      } else {
        webpage += F("<br>Delete failed<br>");
      }
    }
    if (request->argName(i) == "folder") {
      Serial.printf("Removing Dir: %s\n", request->arg(i).c_str());
      webpage += F("<hr>Removing Dir: <br>");
      webpage += request->arg(i);
      if (SD.rmdir(request->arg(i))) {
        webpage += F("<br>Dir removed<br>");
      } else {
        webpage += F("<br>rmdir failed<br>");
      }
    }
  }
  webpage += F("<br><button class='buttons' onclick=\"location.href='/';\">OK</button>");
  webpage += footer;
  request->send(200, "text/html", webpage);
}

void deleteConfirm(AsyncWebServerRequest *request) {
  if (!handleLogin(request))
    return;

  if (webfileReadonly) {
    handleNotFound(request);
    return;
  }

  String webpage = "";
  webpage += header;
  for (uint8_t i = 0; i < request->args(); i++) {
    if (request->argName(i) == "file") {
      webpage += F("<hr>Do you want to delete the file:<br>");
      webpage += request->arg(i);
      webpage += F("<br><br><button class='buttons' onclick=\"location.href='/doDelete?file=");
      webpage += request->arg(i);
      webpage += F("';\">Yes</button>");

    }
    if (request->argName(i) == "folder") {
      webpage += F("<hr>Do you want to delete the Directory:<br>");
      webpage += request->arg(i);
      webpage += F("<br><br><button class='buttons' onclick=\"location.href='/doDelete?folder=");
      webpage += request->arg(i);
      webpage += F("';\">Yes</button>");
    }
  }

  webpage += F("<button class='buttons' onclick='window.history.back();'>No</button>");
  webpage += footer;
  request->send(200, "text/html", webpage);
}

void doFUpload(AsyncWebServerRequest *request) {
  if (!handleLogin(request))
    return;

  if (webfileReadonly) {
    handleNotFound(request);
    return;
  }

  request->send(200);
}

File UploadFile;
size_t UploadTotalSize = 0;
void handleFileUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!handleLogin(request))
    return;

  if (webfileReadonly) {
    handleNotFound(request);
    return;
  }

  // upload a new file to the Filing system
  if (!index)
  {
    String filepath = filename;
    if (!filepath.startsWith("/")) filepath = uploadPath + "/" + filepath;
    Serial.print("Upload File Name: "); Serial.println(filepath);
    if (SD.exists(filepath)) {
      SD.remove(filepath);                         // Remove a previous version, otherwise data is appended the file again
    }
    UploadFile = SD.open(filepath, FILE_WRITE); // Open the file for writing in SPIFFS (create it, if doesn't exist)
    UploadTotalSize = 0;
    filepath = String();
  }
  ; if (len)
  {
    if (UploadFile) {
      UploadFile.write(data, len); // Write the received bytes to the file
      UploadTotalSize += len;
    }
  }
  ; if (final)
  {
    if (UploadFile)         // If the file was successfully created
    {
      UploadFile.close();   // Close the file again
      Serial.print("Upload Size: "); Serial.println(UploadTotalSize);

      String webpage = "";
      webpage += header;
      webpage += F("<hr>File was successfully uploaded<br>");
      webpage += F("Uploaded File Name: ");
      webpage += filename + "<br>";
      webpage += F("File Size: ");
      webpage += file_size(UploadTotalSize) + "<br>";
      webpage += "<button class='buttons' onclick='window.history.back();'>OK</button>";
      webpage += footer;
      request->send(200, "text/html", webpage);
    }
    else
    {
      String webpage = "";
      webpage += header;
      webpage += F("<hr>Could Not Create Uploaded File (write-protected?)<br>");
      webpage += "<button class='buttons' onclick='window.history.back();'>OK</button>";
      webpage += footer;
      request->send(200, "text/html", webpage);
    }
  }
}

void _streamBufCore(AsyncWebServerResponse *response, const size_t Size, const bool isGz, const String & contentType)
{
  if (isGz) {
    response->addHeader("Content-Encoding","gzip");
  }
}

size_t streamBuf(AsyncWebServerRequest *request, uint8_t *Buf, const size_t Size, const String & contentType) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, contentType, Buf, Size);

  _streamBufCore(response, Size, false, contentType);
  request->send(response);
  return Size;
}

File fileReadCur;
size_t streamFile(AsyncWebServerRequest *request, String path, const String & contentType) {
  fileReadCur = SD.open(path, "r");
  size_t Size = fileReadCur.size();

  AsyncWebServerResponse *response = request->beginResponse(contentType, Size, [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
    if (fileReadCur) {
      size_t ret = 0;
      size_t readLen = fileReadCur.available();
      if (maxLen < readLen) {
        readLen = maxLen;
      }
      if (readLen) {
        ret = fileReadCur.read(buffer, readLen);
      }
      if (!fileReadCur.available()) {
        fileReadCur.close();
        Serial.print("File closed.");
      }
      return ret;
    }
    else
      return 0;
  });

  _streamBufCore(response, Size, false, contentType);
  request->send(response);

  return Size;
}

bool handleFileRead(AsyncWebServerRequest *request, String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(request, path);
  String pathWithGz = path + ".gz";
  
  if (SD.exists(pathWithGz) || SD.exists(path)) {
    uint8_t *buf = NULL;
    uint32_t len;

    if (SD.exists(pathWithGz))
      path += ".gz";

    Serial.println("open: " + path);
    streamFile(request, path, contentType);

    Serial.println("File sent.");
    return true;
  }

  return false;
}

void handleScreenImg(AsyncWebServerRequest *request) {
  String contentType = getContentType(request, "/screen.bmp");
  
  streamBuf(request, MG.lcd.GetBitmapBuf(), MG.lcd.GetBitmapSize(), contentType);
}

String htmlColumn(String value, String Params = "") {
  String tree = "";

      tree += F("<td ");
      tree += Params;
      tree += F(" data-value=\"");
      tree += value;
      tree += F("\">");
      tree += value;
      tree += F("</td>");

  return tree;
}

String htmlDetailsColumn(int64_t value, String text = "") {
  String tree = "";

      tree += F("<td class=\"dCol\" data-value=\")");
  if (text == "-")
      tree += "-1";
  else
      tree += file_size(value);
      tree += F("\">");
  if (text != "")
      tree += text;
  else
      tree += file_size(value);
      tree += F("</td>");

  return tree;
}

void FillMemInfo(MemInfo* mi) {
#ifdef ESP32
  mi->SDcard.Size = SD.cardSize();

  mi->SDcard.Free = SD.totalBytes()-SD.usedBytes();
#elif defined(ESP8266)
  mi->SDcard.Size = SD.size64();

  mi->SDcard.Free = 0;
#else
  mi->SDcard.Size = SD.size();

  mi->SDcard.Free = 0;
#endif

  mi->SDcard.MinFree = 0;

  mi->SDcard.MaxAlloc = 0;

  mi->Flash.Size = ESP.getFlashChipSize();

  mi->Flash.Free = ESP.getFreeSketchSpace();

  mi->Flash.MinFree = 0;

  mi->Flash.MaxAlloc = 0;

#ifdef ESP32_WITH_PSRAM
  mi->Psram.Size = ESP.getPsramSize();

  mi->Psram.Free = ESP.getFreePsram();

  mi->Psram.MinFree = ESP.getMinFreePsram();

  mi->Psram.MaxAlloc = ESP.getMaxAllocPsram();
#else
  mi->Psram.Size = 0;

  mi->Psram.Free = 0;

  mi->Psram.MinFree = 0;

  mi->Psram.MaxAlloc = 0;
#endif

#ifdef ESP32
  mi->Heap.Size = ESP.getHeapSize();

  mi->Heap.Free = ESP.getFreeHeap();

  mi->Heap.MinFree = ESP.getMinFreeHeap();

  mi->Heap.MaxAlloc = ESP.getMaxAllocHeap();
#elif defined(ESP8266)
  mi->Heap.Size = 80*1024;

  mi->Heap.Free = ESP.getFreeHeap();

  mi->Heap.MinFree = 0;

  mi->Heap.MaxAlloc = ESP.getMaxFreeBlockSize();
#else
  mi->Heap.Size = 0;

  mi->Heap.Free = 0;

  mi->Heap.MinFree = 0;

  mi->Heap.MaxAlloc = 0;
#endif
}

String htmlMemTable(MemInfo* mi) {
  String webpage = "";

  webpage += F("<table>\n");

  webpage += F("<thead>\n");
  webpage += F("<tr class=\"header\" id=\"theader\">");
  webpage += F("<th onclick=\"sortTable(0);\">Name</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(1);\">Size</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(2);\">Free</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(3);\">MinFree</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(4);\">MaxAlloc</th>");
  webpage += F("</tr>\n");
  webpage += F("</thead>");

  webpage += F("<tr>");

  webpage += htmlColumn("SD");

  webpage += htmlDetailsColumn(mi->SDcard.Size);

  webpage += htmlDetailsColumn(mi->SDcard.Free);

  webpage += htmlDetailsColumn(mi->SDcard.MinFree, "-");

  webpage += htmlDetailsColumn(mi->SDcard.MaxAlloc, "-");

  webpage += F("</tr>\n");

  webpage += F("<tr>");

  webpage += htmlColumn("Flash");

  webpage += htmlDetailsColumn(mi->Flash.Size);

  webpage += htmlDetailsColumn(mi->Flash.Free);

  webpage += htmlDetailsColumn(mi->Flash.MinFree, "-");

  webpage += htmlDetailsColumn(mi->Flash.MaxAlloc, "-");

  webpage += F("</tr>\n");

  webpage += F("<tr>");

  webpage += htmlColumn("Psram");

  webpage += htmlDetailsColumn(mi->Psram.Size);

  webpage += htmlDetailsColumn(mi->Psram.Free);

  webpage += htmlDetailsColumn(mi->Psram.MinFree);

  webpage += htmlDetailsColumn(mi->Psram.MaxAlloc);

  webpage += F("</tr>\n");

  webpage += F("<tr>");

  webpage += htmlColumn("Heap");

  webpage += htmlDetailsColumn(mi->Heap.Size);

  webpage += htmlDetailsColumn(mi->Heap.Free);

  webpage += htmlDetailsColumn(mi->Heap.MinFree);

  webpage += htmlDetailsColumn(mi->Heap.MaxAlloc);

  webpage += F("</tr>\n");

  webpage += F("</table>\n");

  return webpage;
}

void handleSysInfo(AsyncWebServerRequest *request) {
  MemInfo memnow;
  String webpage = "";

  FillMemInfo(&memnow);

  webpage += header;
  webpage.replace("Web FileBrowser", "System Information");

  webpage += F("<table>\n");

  webpage += F("<thead>\n");
  webpage += F("<tr class=\"header\" id=\"theader\">");
  webpage += F("<th onclick=\"sortTable(0);\">Name</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(1);\">Rev</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(2);\">Mac</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(3);\">Flash Speed</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(4);\">Flash Mode</th>");
  webpage += F("</tr>\n");
  webpage += F("</thead>");

  webpage += F("<tr>");

#ifdef ESP32
  webpage += htmlColumn("ESP32");
#elif defined(ESP8266)
  webpage += htmlColumn("ESP8266");
#else
  webpage += htmlColumn("Chip");
#endif

  webpage += F("<td class=\"dCol\" data-value=\"0\">");
#ifdef ESP32
  webpage += ESP.getChipRevision();
#endif
  webpage += F("</td>");

  webpage += F("<td class=\"dCol\" data-value=\"0\">");
#ifdef ESP32
  uint64_t _chipmacid = ESP.getEfuseMac();
  uint8_t *_chipmac = (uint8_t *)&_chipmacid;

  webpage += String(_chipmac[0], HEX);
  webpage += F(":");
  webpage += String(_chipmac[1], HEX);
  webpage += F(":");
  webpage += String(_chipmac[2], HEX);
  webpage += F(":");
  webpage += String(_chipmac[3], HEX);
  webpage += F(":");
  webpage += String(_chipmac[4], HEX);
  webpage += F(":");
  webpage += String(_chipmac[5], HEX);
#endif
  webpage += F("</td>");

  webpage += F("<td class=\"dCol\" data-value=\"0\">");
  webpage += freq_hz(ESP.getFlashChipSpeed());
  webpage += F("</td>");

  webpage += F("<td class=\"dCol\" data-value=\"0\">");

    switch (ESP.getFlashChipMode()) {
      case FM_QIO: // 0x00,
        webpage += F("QIO");
        break;
      case FM_QOUT: // 0x01,
        webpage += F("QOUT");
        break;
      case FM_DIO: // 0x02,
        webpage += F("DIO");
        break;
      case FM_DOUT: // 0x03,
        webpage += F("DOUT");
        break;
#ifdef ESP32
      case FM_FAST_READ: // 0x04,
        webpage += F("Fast read");
        break;
      case FM_SLOW_READ: // 0x05,
        webpage += F("Slow read");
        break;
#endif
      case FM_UNKNOWN: // 0xff
        webpage += F("Unknown");
        break;
      default:
        break;
    }

  webpage += F("</td>");

  webpage += F("</tr>\n");

  webpage += F("</table>\n");

  webpage += F("<p>Memory on boot:</p>\n");

  webpage += htmlMemTable(&memboot);

  webpage += F("<p>Memory now:</p>\n");

  webpage += htmlMemTable(&memnow);

  webpage += F("<script>\n");
  webpage += F("function add_time(link)\n");
  webpage += F("{\n");
  webpage += F("  location.href=link.href + '&time=' + new Date().getTime();\n");
  webpage += F("  return false;\n");
  webpage += F("}\n");
  webpage += F("</script>\n");

  webpage += footer;

  webpage.replace("WiFi Web - ", "<a href=\"/\">Home</a> - <a href=\"/syssetup?page=7\" onclick=\"return add_time(this);\">Setup</a> - WiFi Web - ");

  request->send(200, "text/html", webpage);
}

bool GetWifiAPData(bool write = false) {
  String path = "/WIFIAP.TXT";

  Serial.print("Reading file: ");
  Serial.println(path);
  File wifiAPFile = SD.open(path);

  if (!wifiAPFile) {
    Serial.println("Failed to open file for reading");

    if (!write) {
      return false;
    }

    File wifiAPFile = SD.open(path, FILE_WRITE);
    if (!wifiAPFile) {
      Serial.println("Failed to open file for writing");
    } else {
      WifiAPSSID+="-";
      WifiAPSSID+=String(random(0xffffff),HEX);
      WifiAPPSK+="-";
      WifiAPPSK+=String(random(65535),HEX);

      wifiAPFile.println(WifiAPSSID);
      wifiAPFile.println(WifiAPPSK);

      wifiAPFile.println(WifiAPIP.toString());
      wifiAPFile.println(WifiAPMask.toString());
      wifiAPFile.println(WifiAPGW.toString());

      wifiAPFile.close();
    }
  } else {
    Serial.println("Read from file: ");
    while (wifiAPFile.available()) {
      WifiAPSSID = wifiAPFile.readStringUntil('\n');
      WifiAPSSID.replace("\r", "");
      WifiAPPSK = wifiAPFile.readStringUntil('\n');
      WifiAPPSK.replace("\r", "");

      String WifiAPIPValue   = WifiAPIP.toString();
      String WifiAPMaskValue = WifiAPMask.toString();
      String WifiAPGWValue   = WifiAPGW.toString();

      if (wifiAPFile.available()) {
        WifiAPIPValue = wifiAPFile.readStringUntil('\n');
        WifiAPIPValue.replace("\r", "");
        WifiAPGWValue = WifiAPIPValue;
      }
      if (wifiAPFile.available()) {
        WifiAPMaskValue = wifiAPFile.readStringUntil('\n');
        WifiAPMaskValue.replace("\r", "");
      }
      if (wifiAPFile.available()) {
        WifiAPGWValue = wifiAPFile.readStringUntil('\n');
        WifiAPGWValue.replace("\r", "");
      }
      if (wifiAPFile.available()) {
        WifiAPHost = wifiAPFile.readStringUntil('\n');
        WifiAPHost.replace("\r", "");
      }

      WifiAPIP.fromString(WifiAPIPValue);
      WifiAPMask.fromString(WifiAPMaskValue);
      WifiAPGW.fromString(WifiAPGWValue);

      break;
    }
    wifiAPFile.close();
  }

  return true;
}

bool GetWifiData() {
  String path = "/WIFI.TXT";

  Serial.print("Reading file: ");
  Serial.println(path);
  File wifiFile = SD.open(path);

  if (!wifiFile) {
    Serial.println("Failed to open file for reading");
    return false;
  } else if (!wifiFile.available()) {
    Serial.println("Empty file");
    return false;
  } else {
    int wifiIndex = 0;

    if (WifiList) {
      delete [] WifiList;
      WifiList = NULL;
    }
    WifiCount = 0;

    /* Todo: find place to store client mode hostname */
    WifiHost = WifiAPHost;

    Serial.println("Read from file: ");
    while (wifiFile.available()) {
      WifiSSID = wifiFile.readStringUntil('\n');
      WifiSSID.replace("\r", "");
      if (!wifiFile.available()) {
        break;
      }
      WifiPSK = wifiFile.readStringUntil('\n');
      WifiPSK.replace("\r", "");
      WifiCount += 1;
    }
    wifiFile.close();

    if (!WifiCount) {
      return false;
    }

    WifiList = new WifiData[WifiCount];

    wifiFile = SD.open(path);
    while (wifiFile.available()) {
      if (wifiIndex < WifiCount) {
        WifiList[wifiIndex].SSID = wifiFile.readStringUntil('\n');
        WifiList[wifiIndex].SSID.replace("\r", "");
        WifiList[wifiIndex].PSK = wifiFile.readStringUntil('\n');
        WifiList[wifiIndex].PSK.replace("\r", "");
        wifiIndex += 1;
      } else {
        break;
      }
    }
    wifiFile.close();

    WifiSSID = WifiList[0].SSID;
    WifiPSK = WifiList[0].PSK;
  }

  return true;
}

bool GetWifiIPData(String SSID) {
  String path = "/WIFIIP-";

  path += SSID;
  path += ".TXT";

  Serial.print("Reading file: ");
  Serial.println(path);
  File wifiIPFile = SD.open(path);

  if (!wifiIPFile) {
    Serial.println("Failed to open file for reading");
    return false;
  } else if (!wifiIPFile.available()) {
    Serial.println("Empty file");
    return false;
  } else {
    Serial.println("Read from file: ");
    while (wifiIPFile.available()) {
      String WifiIPValue   = WifiIP.toString();
      String WifiMaskValue = WifiMask.toString();
      String WifiGWValue   = WifiGW.toString();
      String WifiDNS1Value = WifiDNS1.toString();
      String WifiDNS2Value = WifiDNS2.toString();

      if (wifiIPFile.available()) {
        WifiIPValue = wifiIPFile.readStringUntil('\n');
        WifiIPValue.replace("\r", "");
        WifiGWValue = "0.0.0.0";
      }
      if (wifiIPFile.available()) {
        WifiMaskValue = wifiIPFile.readStringUntil('\n');
        WifiMaskValue.replace("\r", "");
      }
      if (wifiIPFile.available()) {
        WifiGWValue = wifiIPFile.readStringUntil('\n');
        WifiGWValue.replace("\r", "");
      }

      if (wifiIPFile.available()) {
        WifiDNS1Value = wifiIPFile.readStringUntil('\n');
        WifiDNS1Value.replace("\r", "");
      }
      if (wifiIPFile.available()) {
        WifiDNS2Value = wifiIPFile.readStringUntil('\n');
        WifiDNS2Value.replace("\r", "");
      }

      WifiIP.fromString(WifiIPValue);
      WifiMask.fromString(WifiMaskValue);
      WifiGW.fromString(WifiGWValue);
      WifiDNS1.fromString(WifiDNS1Value);
      WifiDNS2.fromString(WifiDNS2Value);

      break;
    }
    wifiIPFile.close();
  }

  return true;
}

void GetNewWifiWebMode() {
  if (NewWifiWebMode == 0xFF) {
    NewWifiWebMode = WifiWebMode;
    if (NewWifiWebMode == 1) {
      NewWifiAPSSID = WifiAPSSID;
      NewWifiAPPSK  = WifiAPPSK;
    }
    else if (NewWifiWebMode == 2) {
      NewWifiSSID = WifiSSID;
      NewWifiPSK  = WifiPSK;
    }
  }

  if (NewWifiAPSSID == "") {

    if (WifiAPSSID == "odroidgo") {
      GetWifiAPData();
    }

    NewWifiAPSSID = WifiAPSSID;
    NewWifiAPPSK  = WifiAPPSK;
  }

  if (NewWifiSSID == "") {

    if (WifiSSID == "") {
      GetWifiData();
    }

    NewWifiSSID = WifiSSID;
    NewWifiPSK  = WifiPSK;
  }

  if (NewWifiIPMode == 0xFF) {
    NewWifiIPMode = WifiIPMode;
  }

  if (NewIPMode == 0xFF) {
    if (NewWifiWebMode == 2) {
      NewIPMode = NewWifiIPMode;
    } else {
      NewIPMode = 1;
    }
  }

  if (NewWifiAPIP == IPAddress(0, 0, 0, 0)) {
    NewWifiAPIP   = WifiAPIP;
    NewWifiAPMask = WifiAPMask;
    NewWifiAPGW   = WifiAPGW;
  }

  if (NewWifiIP == IPAddress(0, 0, 0, 0)) {
    NewWifiIP   = WifiIP;
    NewWifiMask = WifiMask;
    NewWifiGW   = WifiGW;
    NewWifiDNS1 = WifiDNS1;
    NewWifiDNS2 = WifiDNS2;
  }

  if (NewWifiAPHost == "") {
    NewWifiAPHost = WifiAPHost;
  }

  if (NewWifiHost == "") {
    NewWifiHost = WifiHost;
  }
}

time_t getBrowserTime(const String value) {
  if (value.length() > 3) {
    /* time_t = number of seconds since 00:00, Jan 1 1970 UTC
       javascript time = number of milliseconds since Jan 1 1970 */
    return value.substring(0, value.length() - 3).toInt();
  }

  return 0;
}

IPAddress calculateCIDRSubnet(uint8_t CIDR) {
        uint32_t mask = 0;

        while (CIDR--) {
	    mask |= 1 << CIDR;
	}

	return IPAddress(mask);
}

String htmlWifi(uint8_t settingsPage) {
  String webpage = "";

  webpage += F("<p>");
  webpage += F("WiFi:");

  if (settingsPage == 1) {
    webpage += F("<a href=\"/syssetup?page=2\" onclick=\"return add_time(this);\">Web</a>\n");
    webpage += F("<a href=\"/syssetup?page=4\" onclick=\"return add_time(this);\">Time</a>\n");
  }

  webpage += F("</p>\n");

  webpage += F("<fieldset>\n");

  webpage += F("  Mode:\n");
  webpage += F("  <input type=\"radio\" onclick='updateWifi(1);' id=\"wifiap\" name=\"wifimode\" value=\"1\"");
  if (NewWifiWebMode == 1)
    webpage += F(" checked=\"checked\"");
  webpage += F(">\n");
  webpage += F("  <label for=\"wifiap\"> AP</label>\n");
  webpage += F("  <input type=\"radio\" onclick='updateWifi(2);' id=\"wificlient\" name=\"wifimode\" value=\"2\"");
  if (NewWifiWebMode == 2)
    webpage += F(" checked=\"checked\"");
  webpage += F(">\n");
  webpage += F("  <label for=\"wificlient\"> Client</label>\n");
  webpage += F("  <input type=\"radio\" onclick='updateWifi(0);' id=\"wifidisabled\" name=\"wifimode\" value=\"0\"");
  if (NewWifiWebMode == 0)
    webpage += F(" checked=\"checked\"");
  webpage += F(">\n");
  webpage += F("  <label for=\"wifidisabled\"> Disabled</label>\n");

  webpage += F("  <br><br>\n");

  webpage += F("  <label for=\"wifissid\">SSID: </label>\n");
  webpage += F("  <input name=\"wifissid\" id=\"wifissid\" value=\"");
  if (NewWifiWebMode == 1)
    webpage += NewWifiAPSSID;
  else if (NewWifiWebMode == 2)
    webpage += NewWifiSSID;
  webpage += F("\">\n");
  webpage += F("\n");
  webpage += F("  <label for=\"wifipsk\">PSK: </label>\n");
  webpage += F("  <input name=\"wifipsk\" id=\"wifipsk\" type=\"password\">\n");
  webpage += F("\n");

  webpage += F("  <br><br>\n");

  webpage += F("  Mode:\n");
  webpage += F("  <input type=\"radio\" onclick='updateIP(1);' id=\"ipstatic\" name=\"ipmode\" value=\"1\"");
  if (NewIPMode == 1)
    webpage += F(" checked=\"checked\"");
  webpage += F(">\n");
  webpage += F("  <label for=\"ipstatic\"> Static</label>\n");
  webpage += F("  <input type=\"radio\" onclick='updateIP(2);' id=\"ipdhcp\" name=\"ipmode\" value=\"2\"");
  if (NewIPMode == 2)
    webpage += F(" checked=\"checked\"");
  webpage += F(">\n");
  webpage += F("  <label for=\"ipdhcp\"> DHCP</label>\n");

  webpage += F("  <br><br>\n");

  webpage += F("  <label for=\"wifiip\">IP: </label>\n");
  webpage += F("  <input name=\"wifiip\" id=\"wifiip\" value=\"");
  String myIPvalue = "";
  if (NewWifiWebMode == 1) {
    myIPvalue = NewWifiAPIP.toString();
  } else if (NewWifiWebMode == 2) {
    myIPvalue = NewWifiIP.toString();
  }
  webpage += myIPvalue;
  webpage += F("\">\n");

  webpage += F("  <label for=\"wifimask\"> / </label>\n");
  webpage += F("  <input name=\"wifimask\" id=\"wifimask\" value=\"");
  if (NewWifiWebMode == 1) {
    myIPvalue = NewWifiAPMask.toString();
  } else if (NewWifiWebMode == 2) {
    myIPvalue = NewWifiMask.toString();
  }
  webpage += myIPvalue;
  webpage += F("\">\n");

  webpage += F("  <label for=\"wifigw\">Gateway: </label>\n");
  webpage += F("  <input name=\"wifigw\" id=\"wifigw\" value=\"");
  if (NewWifiWebMode == 1) {
    myIPvalue = NewWifiAPGW.toString();
  } else if (NewWifiWebMode == 2) {
    myIPvalue = NewWifiGW.toString();
  }
  webpage += myIPvalue;
  webpage += F("\">\n");

  webpage += F("  <br><br>\n");

  webpage += F("  <label for=\"wifidns1\">DNS1: </label>\n");
  webpage += F("  <input name=\"wifidns1\" id=\"wifidns1\" value=\"");
  if (NewWifiWebMode == 1) {
    myIPvalue = NewWifiAPIP.toString();
  } else if (NewWifiWebMode == 2) {
    myIPvalue = NewWifiDNS1.toString();
  }
  webpage += myIPvalue;
  webpage += F("\">\n");

  webpage += F("  <label for=\"wifidns2\">DNS2: </label>\n");
  webpage += F("  <input name=\"wifidns2\" id=\"wifidns2\" value=\"");
  if (NewWifiWebMode == 1) {
    myIPvalue = "";
  } else if (NewWifiWebMode == 2) {
    myIPvalue = NewWifiDNS2.toString();
  }
  webpage += myIPvalue;
  webpage += F("\">\n");

  webpage += F("  <label for=\"wifihost\">Hostname: </label>\n");
  webpage += F("  <input name=\"wifihost\" id=\"wifihost\" value=\"");
  if (NewWifiWebMode == 1)
    webpage += NewWifiAPHost;
  else if (NewWifiWebMode == 2)
    webpage += NewWifiHost;
  webpage += F("\">\n");

  webpage += F("</fieldset>\n");

  return webpage;
}

String htmlWebFile(uint8_t settingsPage) {
  String webpage = "";

  webpage += F("<p>");

  if (settingsPage == 2) {
    webpage += F("<a href=\"/syssetup?page=1\" onclick=\"return add_time(this);\">WiFi</a>\n");
  }
  webpage += F("Web:");
  if (settingsPage == 2) {
    webpage += F("<a href=\"/syssetup?page=4\" onclick=\"return add_time(this);\">Time</a>\n");
  }

  webpage += F("</p>\n");

  webpage += F("<fieldset>\n");

  webpage += F("  <label for=\"webuser\">Username: </label>\n");
  webpage += F("  <input name=\"webuser\" id=\"webuser\" value=\"");
  webpage += WifiWebUser;
  webpage += F("\">\n");
  webpage += F("\n");
  webpage += F("  <label for=\"webpass\">Password: </label>\n");
  webpage += F("  <input name=\"webpass\" id=\"webpass\" type=\"password\">\n");
  webpage += F("\n");

  webpage += F("  <br><br>\n");

  webpage += F("  WebFile Mode:\n");
  webpage += F("  <input type=\"radio\" onclick='updateWebFile(1);' id=\"webfilerw\" name=\"webfilemode\" value=\"2\"");
  if (!webfileReadonly && !webfileDisabled)
    webpage += F(" checked=\"checked\"");
  webpage += F(">\n");
  webpage += F("  <label for=\"webfilerw\"> Read/Write</label>\n");
  webpage += F("  <input type=\"radio\" onclick='updateWebFile(2);' id=\"webfilereadonly\" name=\"webfilemode\" value=\"1\"");
  if (webfileReadonly && !webfileDisabled)
    webpage += F(" checked=\"checked\"");
  webpage += F(">\n");
  webpage += F("  <label for=\"webfilereadonly\"> Read Only</label>\n");
  webpage += F("  <input type=\"radio\" onclick='updateWebFile(0);' id=\"webfiledisabled\" name=\"webfilemode\" value=\"0\"");
  if (webfileReadonly && webfileDisabled)
    webpage += F(" checked=\"checked\"");
  webpage += F(">\n");
  webpage += F("  <label for=\"webfiledisabled\"> Disabled</label>\n");
/*
  webpage += F("  <input type=\"checkbox\" id=\"webrcautorefresh\" name=\"webrcautorefresh\" value=\"1\"");
  if (webrcAutoRefresh)
    webpage += F(" checked=\"checked\"");
  webpage += F(">\n");
  webpage += F("  <label for=\"webrcautorefresh\"> Auto Refresh</label>\n");
*/
  webpage += F("</fieldset>\n");

  return webpage;
}

String htmlTime(uint8_t settingsPage) {
  String webpage = "";

  webpage += F("<p>");

  if (settingsPage == 4) {
    webpage += F("<a href=\"/syssetup?page=1\" onclick=\"return add_time(this);\">WiFi</a>\n");
    webpage += F("<a href=\"/syssetup?page=2\" onclick=\"return add_time(this);\">Web</a>\n");
  }
  webpage += F("Time:");

  webpage += F("</p>\n");

  webpage += F("<fieldset>\n");

  webpage += F("  Current local time: ");
  webpage += getLocalTimeString();
  webpage += F("  <br>\n");

  if (browserTime) {
    webpage += F("  Browser local time: ");
    webpage += getBrowserTimeString();
    webpage += F("  <br>\n");
  }
  webpage += F("  <br>\n");

  webpage += F("  <label for=\"ntpserver1\">NTP Server 1: </label>\n");
  webpage += F("  <input name=\"ntpserver1\" id=\"ntpserver1\" value=\"");
  webpage += ntpServer1;
  webpage += F("\">\n");

  webpage += F("  <label for=\"ntpserver2\">NTP Server 2: </label>\n");
  webpage += F("  <input name=\"ntpserver2\" id=\"ntpserver2\" value=\"");
  webpage += ntpServer2;
  webpage += F("\">\n");

  webpage += F("  <label for=\"ntpserver3\">NTP Server 3: </label>\n");
  webpage += F("  <input name=\"ntpserver3\" id=\"ntpserver3\" value=\"");
  webpage += ntpServer3;
  webpage += F("\">\n");

  webpage += F("  <br><br>\n");

  webpage += F("  <label for=\"gmtoffset\">GMT Offset: </label>\n");
  webpage += F("  <input name=\"gmtoffset\" id=\"gmtoffset\" value=\"");
  webpage += String(gmtOffset_sec);
  webpage += F("\">\n");

  webpage += F("  <label for=\"dstoffset\">DST Offset: </label>\n");
  webpage += F("  <input name=\"dstoffset\" id=\"dstoffset\" value=\"");
  webpage += String(daylightOffset_sec);
  webpage += F("\">\n");

  webpage += F("  <button class='buttons' onclick='return updateQuorum();' name='quorum'>Quorum</button>\n");

  webpage += F("</fieldset>\n");

  return webpage;
}

String htmlWiFiNeighbours(uint8_t settingsPage) {
  String webpage = "";

  webpage += F("<p>WiFi Neighbours:</p>\n");
  webpage += F("<table>\n");

  webpage += F("<thead>\n");
  webpage += F("<tr class=\"header\" id=\"wntheader\">");
  webpage += F("<th onclick=\"sortTable(0);\">SSID</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(1);\">Channel</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(2);\">RSSI</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(3);\">Encryption</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(4);\">BSSID</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(5);\">Known</th>");
  webpage += F("</tr>\n");
  webpage += F("</thead>");

  webpage += F("<tbody id=\"wntbody\">");

  int16_t count = WiFi.scanNetworks();

  for (int16_t i = 0; i < count; i++) {
    webpage += F("<tr>");

    webpage += htmlColumn(WiFi.SSID(i), "onclick=\"updateSSID("+String(i)+");\"");

    webpage += F("<td class=\"dCol\" data-value=\"0\">");
    webpage += WiFi.channel(i);
    webpage += F("</td>");

    webpage += F("<td class=\"dCol\" data-value=\"0\">");
    webpage += WiFi.RSSI(i);
    webpage += F("</td>");

    webpage += F("<td class=\"dCol\" data-value=\"0\">");

    switch (WiFi.encryptionType(i)) {
      case WIFI_AUTH_OPEN:             /**< authenticate mode : open */
        webpage += F("Open");
        break;
      case WIFI_AUTH_WEP:              /**< authenticate mode : WEP */
        webpage += F("WEP");
        break;
      case WIFI_AUTH_WPA_PSK:          /**< authenticate mode : WPA_PSK */
        webpage += F("WPA PSK");
        break;
      case WIFI_AUTH_WPA2_PSK:         /**< authenticate mode : WPA2_PSK */
        webpage += F("WPA2 PSK");
        break;
      case WIFI_AUTH_WPA_WPA2_PSK:     /**< authenticate mode : WPA_WPA2_PSK */
        webpage += F("WPA/WPA2 PSK");
        break;
#ifdef ESP32
      case WIFI_AUTH_WPA2_ENTERPRISE:  /**< authenticate mode : WPA2_ENTERPRISE */
        webpage += F("WPA2 Enterprise");
        break;
#endif
      default:
        break;
    }

    webpage += F("</td>");

    webpage += F("<td class=\"dCol\" data-value=\"0\">");
    webpage += WiFi.BSSIDstr(i);
    webpage += F("</td>");

    webpage += F("<td class=\"dCol\" data-value=\"0\">");

    int wifiIndex = 0;
    while (wifiIndex < WifiCount) {
      if (WifiList[wifiIndex].SSID == WiFi.SSID(i)) {
        webpage += F("yes");
        break;
      }
      wifiIndex++;
    }

    webpage += F("</td>");

    webpage += F("</tr>\n");
  }

  webpage += F("</tbody>\n");

  webpage += F("</table>\n");

  return webpage;
}

void handleSysSetup(AsyncWebServerRequest *request) {
  MemInfo memnow;
  bool needRestart = false;
  String webpage = "";
  uint8_t settingsPage = 7;

  if (!handleLogin(request))
    return;

  FillMemInfo(&memnow);

  GetNewWifiWebMode();

  for (uint8_t i = 0; i < request->args(); i++) {
    if (request->argName(i) == "time") {
      browserTime = getBrowserTime(request->arg(i));
    }
    else if (request->argName(i) == "page") {
      settingsPage = request->arg(i).toInt();
    }
  }

  if ((settingsPage == 7) && (memnow.Heap.MaxAlloc < 49152)) {
    settingsPage = 1;
  }

    Serial.print("settingsPage: ");
    Serial.println(settingsPage);

  webpage = "";
  webpage += header;

  webpage.replace("Web FileBrowser", "System Setup");

  webpage += F("<script>\n");
  webpage += F("var WifiWebMode = ");
  webpage += String(NewWifiWebMode);
  webpage += F(";\n");

  webpage += F("var WifiAPSSID = \"");
  webpage += NewWifiAPSSID;
  webpage += F("\";\n");
  webpage += F("var WifiAPPSK = \"");
  webpage += F("\";\n");

  webpage += F("var WifiAPIP = \"");
  webpage += NewWifiAPIP.toString();
  webpage += F("\";\n");
  webpage += F("var WifiAPMask = \"");
  webpage += NewWifiAPMask.toString();
  webpage += F("\";\n");
  webpage += F("var WifiAPGW = \"");
  webpage += NewWifiAPGW.toString();
  webpage += F("\";\n");

  webpage += F("var WifiAPHost = \"");
  webpage += NewWifiAPHost;
  webpage += F("\";\n");

  webpage += F("var WifiSSID = \"");
  webpage += NewWifiSSID;
  webpage += F("\";\n");
  webpage += F("var WifiPSK = \"");
  webpage += F("\";\n");

  webpage += F("var WifiIP = \"");
  webpage += NewWifiIP.toString();
  webpage += F("\";\n");
  webpage += F("var WifiMask = \"");
  webpage += NewWifiMask.toString();
  webpage += F("\";\n");
  webpage += F("var WifiGW = \"");
  webpage += NewWifiGW.toString();
  webpage += F("\";\n");

  webpage += F("var WifiDNS1 = \"");
  webpage += NewWifiDNS1.toString();
  webpage += F("\";\n");
  webpage += F("var WifiDNS2 = \"");
  webpage += NewWifiDNS2.toString();
  webpage += F("\";\n");

  webpage += F("var WifiHost = \"");
  webpage += NewWifiHost;
  webpage += F("\";\n");

  webpage += F("var WifiIPMode = ");
  webpage += String(NewWifiIPMode);
  webpage += F(";\n");

  if (settingsPage & 1) {
    webpage += setwifi_script;
  }

  webpage += settings_script;

  webpage += F("</script>\n");

  webpage += F("<form action='/syssetup' method='post' enctype='multipart/form-data'>\n");

  webpage += F("<input name=\"page\" id=\"page\" type=\"hidden\" value=\"");
  webpage += settingsPage;
  webpage += F("\">\n");

  webpage += F("<br>\n");
  webpage += F("<input name=\"time\" id=\"time\" type=\"hidden\">\n");

  if (settingsPage & 1) {
    webpage += htmlWifi(settingsPage);
  }

  if (settingsPage & 2) {
    webpage += htmlWebFile(settingsPage);
  }

  if (settingsPage & 4) {
    webpage += htmlTime(settingsPage);
  }

  webpage += F("<br>");

  webpage += F("<button class='buttons' type='submit' onclick='updateTime();' name='action' value='Apply'>Apply</button>\n");
  webpage += F("<button class='buttons' type='submit' onclick='updateTime();' name='action' value='Save'>Save</button>\n");
  if ((browserTime) && (settingsPage & 4)) {
    webpage += F("<button class='buttons' type='submit' onclick='updateTime();' name='action' value='Sync'>Sync Time</button>\n");
  }

  webpage += F("</form>\n");

  if (NewWifiWebMode != WifiWebMode) {
    needRestart = true;
  } else if (NewWifiWebMode == 1) {
    if ((NewWifiAPSSID != WifiAPSSID) || (NewWifiAPPSK != WifiAPPSK)) {
      needRestart = true;
    }
  } else if (NewWifiWebMode == 2) {
    if ((NewWifiSSID != WifiSSID) || (NewWifiPSK != WifiPSK)) {
      needRestart = true;
    }
  }

  if (needRestart) {
    webpage += F("  <p>You need to restart device to apply settings.</p>\n");
  }

  if ((settingsPage & 1) && (memnow.Heap.MaxAlloc >= 49152)) {
    webpage += htmlWiFiNeighbours(settingsPage);
  }

  webpage += F("<script>\n");

  webpage += F("updateWifi(");
  webpage += NewWifiWebMode;
  webpage += F(");\n");

  webpage += F("</script>\n");

  webpage += footer;

  Serial.print("Page: ");
  Serial.print(webpage.length());
  Serial.print(" MaxAlloc: ");
  Serial.println(bm_max_alloc());

  webpage.replace("WiFi Web - ", "<a href=\"/sysinfo\">System Information</a> - <a href=\"/\">Home</a> - WiFi Web - ");

  request->send(200, "text/html", webpage);
}

void doSaveSetup(AsyncWebServerRequest *request) {
  if (!handleLogin(request))
    return;

  String webpage = "";
  webpage += header;
  int8_t wifimode = -1;
  String wifissid = "";
  String wifipsk = "";
  int8_t ipmode = -1;
  IPAddress wifiip;
  IPAddress wifimask;
  IPAddress wifigw;
  IPAddress wifidns1;
  IPAddress wifidns2;
  String wifihost = "";
  String webuser = "";
  String webpass = "";
  String action = "";
  int8_t webfilemode = -1;
  bool doAutoRefresh = webrcAutoRefresh;
  String new_ntpServer1 = ntpServer1;
  String new_ntpServer2 = ntpServer2;
  String new_ntpServer3 = ntpServer3;
  long  new_gmtOffset = gmtOffset_sec;
  int   new_dstOffset = daylightOffset_sec;

  for (uint8_t i = 0; i < request->args(); i++) {
    if (request->argName(i) == "wifimode") {
      wifimode =  request->arg(i).toInt();
    }
    else if (request->argName(i) == "wifissid") {
      wifissid = request->arg(i);
    }
    else if (request->argName(i) == "wifipsk") {
      wifipsk = request->arg(i);
    }

    else if (request->argName(i) == "ipmode") {
      ipmode = request->arg(i).toInt();
    }
    else if (request->argName(i) == "wifiip") {
      wifiip.fromString(request->arg(i));
    }
    else if (request->argName(i) == "wifimask") {
      wifimask.fromString(request->arg(i));
    }
    else if (request->argName(i) == "wifigw") {
      wifigw.fromString(request->arg(i));
    }
    else if (request->argName(i) == "wifidns1") {
      wifidns1.fromString(request->arg(i));
    }
    else if (request->argName(i) == "wifidns2") {
      wifidns2.fromString(request->arg(i));
    }
    else if (request->argName(i) == "wifihost") {
      wifihost = request->arg(i);
    }

    else if (request->argName(i) == "webuser") {
      webuser = request->arg(i);
    }
    else if (request->argName(i) == "webpass") {
      webpass = request->arg(i);
    }

    else if (request->argName(i) == "webfilemode") {
      webfilemode = request->arg(i).toInt();
    }
    else if (request->argName(i) == "webrcautorefresh") {
      doAutoRefresh = request->arg(i).toInt();
    }

    else if (request->argName(i) == "ntpserver1") {
      new_ntpServer1 = request->arg(i);
    }
    else if (request->argName(i) == "ntpserver2") {
      new_ntpServer2 = request->arg(i);
    }
    else if (request->argName(i) == "ntpserver3") {
      new_ntpServer3 = request->arg(i);
    }
    else if (request->argName(i) == "gmtoffset") {
      new_gmtOffset = request->arg(i).toInt();
    }
    else if (request->argName(i) == "dstoffset") {
      new_dstOffset = request->arg(i).toInt();
    }

    else if (request->argName(i) == "time") {
      browserTime = getBrowserTime(request->arg(i));
    }

    else if (request->argName(i) == "action") {
      action = request->arg(i);
    }

  }

  if (action == "Sync") {
    //setTime(browserTime);
    struct timeval now = { .tv_sec = browserTime };
    settimeofday(&now, NULL);

  } else if (action != "") {

    if ((wifimode == 1) && (wifissid != "")) {
      if ((wifipsk == "") && (wifissid == NewWifiAPSSID)) {
        wifipsk = NewWifiAPPSK;
      }
      if ((wifipsk == "") && (wifissid == WifiAPSSID)) {
        wifipsk = WifiAPPSK;
      }

      if (wifipsk != "") {
        NewWifiWebMode = wifimode;
        NewWifiAPSSID  = wifissid;
        NewWifiAPPSK   = wifipsk;
      }

      if (ipmode == 1) {
        NewWifiAPIP   = wifiip;
        NewWifiAPMask = wifimask;
        NewWifiAPGW   = wifigw;
      }

      NewWifiAPHost = wifihost;
    } else if ((wifimode == 2) && (wifissid != "")) {
      if ((wifipsk == "") && (wifissid == NewWifiSSID)) {
        wifipsk = NewWifiPSK;
      }
      if ((wifipsk == "") && (wifissid == WifiSSID)) {
        wifipsk = WifiPSK;
      }

      if (wifipsk == "") {
        int wifiIndex = 0;
        while (wifiIndex < WifiCount) {
          if (wifissid = WifiList[wifiIndex].SSID) {
            wifipsk = WifiList[wifiIndex].PSK;
            break;
          }
          wifiIndex++;
        }
      }

      if (wifipsk != "") {
        NewWifiWebMode = wifimode;
        NewWifiSSID = wifissid;
        NewWifiPSK = wifipsk;
        NewWifiIPMode = 2;
      }

      if (ipmode == 1) {
        NewWifiIPMode = ipmode;
        NewWifiIP   = wifiip;
        NewWifiMask = wifimask;
        NewWifiGW   = wifigw;
        NewWifiDNS1 = wifidns1;
        NewWifiDNS2 = wifidns2;
      }

      NewWifiHost = wifihost;
    } else if (wifimode == 0) {
      NewWifiWebMode = wifimode;
    }

    if ((webuser != "") && (webpass != "")) {
      WifiWebUser = webuser;
      WifiWebPass = webpass;
    }

    if (webfilemode >= 0) {
      webfileDisabled = (webfilemode == 0);
      webfileReadonly = ((webfilemode == 1) || webfileDisabled);
    }

    webrcAutoRefresh = doAutoRefresh;

    if ((new_ntpServer1 != ntpServer1) ||
        (new_ntpServer2 != ntpServer2) ||
        (new_ntpServer3 != ntpServer3) ||
        (new_gmtOffset != gmtOffset_sec) ||
        (new_dstOffset != daylightOffset_sec)) {
      ntpServer1 = new_ntpServer1;
      if ((new_ntpServer2 != "") && (new_ntpServer3 != "")) {
        ntpServer2 = new_ntpServer2;
        ntpServer3 = new_ntpServer3;
      } else {
        ntpServer2 = "";
        ntpServer3 = "";
      }
      gmtOffset_sec = new_gmtOffset;
      daylightOffset_sec = new_dstOffset;

      if (WifiWebMode == 2) {
        if (ntpServer2 != "") {
          configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1.c_str(), ntpServer2.c_str(), ntpServer3.c_str());
        } else {
          configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1.c_str());
        }
      }
    }
  }

  if (action == "Save") {
    SaveWifiWebMode();
  }

  handleSysSetup(request);
}

String getLastElement(String directory)
{
  int i, count;
  for (i = 0, count = 0; directory[i]; i++)
    count += (directory[i] == '/');
  count++;

  int parserCnt = 0;
  int rFromIndex = 0, rToIndex = -1;
  String retElement = "";
  String tempElement = "";
  String path = directory;
  path.remove(0, 1);
  path += "/";
  while (count >= parserCnt) {
    rFromIndex = rToIndex + 1;
    rToIndex = path.indexOf('/', rFromIndex);
    if (count == parserCnt) {
      if (rToIndex == 0 || rToIndex == -1) break;
      tempElement = retElement;
      retElement = path.substring(rFromIndex, rToIndex);
    } else parserCnt++;
  }

  return retElement;
}

String removeLastElement(String directory)
{
  int i, count;
  for (i = 0, count = 0; directory[i]; i++)
    count += (directory[i] == '/');
  count++;

  int parserCnt = 0;
  int rFromIndex = 0, rToIndex = -1;
  String retElement = directory;
  String tempElement = "";
  String path = directory;
  path.remove(0, 1);
  path += "/";
  while (count >= parserCnt) {
    rFromIndex = rToIndex + 1;
    rToIndex = path.indexOf('/', rFromIndex);
    if (count == parserCnt) {
      if (rToIndex == 0 || rToIndex == -1) break;
      tempElement = retElement;
      retElement = "/"+path.substring(0, rFromIndex);
    } else parserCnt++;
  }

  return retElement;
}

String htmlButtonPress(String Caption, String Value, String Params = "class='buttons'") {
  String webpage = "";
  webpage += F("<button ");
  webpage += Params;
  webpage += F(" id='press-");
  webpage += Value;
  webpage += F("' onclick=\"press('");
  webpage += Value;
  webpage += F("');\">");
  webpage += Caption;
  webpage += F("</button>");
  return webpage;
}

String htmlButtonDirect(String Caption, String Value, String Params = "class='buttons'") {
  String webpage = "";
  webpage += F("<button ");
  webpage += Params;
  webpage += F(" id='press-");
  webpage += Value;
  webpage += F("' onclick=\"location.href='/button?press=");
  webpage += Value;
  webpage += F("';\">");
  webpage += Caption;
  webpage += F("</button>");
  return webpage;
}

String htmlDirEntry(File entry, bool shortUrl) {
  String entryName = "";
  String entryPath = "";
  String tree = "";

    entryPath = entry.name();
    entryName = getLastElement(entryPath);

    time_t t = entry.getLastWrite();
    struct tm * tmstruct = localtime(&t);
    String entryTime = getTimeString(t);

    if (entry.isDirectory()) {
      tree += F("<tr>");
      tree += F("<td data-value=\"");
      tree += entryName+"/";
      
      tree += F("\"><a class=\"icon dir\" href=\"");
      if (shortUrl)
        tree += entryName+"/";
      else
        tree += entryPath+"/";
      tree += F("\">");

      tree += F("&nbsp;&nbsp;&nbsp;&nbsp;");

      tree += F("</a><a href=\"");
      if (shortUrl)
        tree += entryName+"/";
      else
        tree += entryPath+"/";
      tree += F("\">");
      tree += entryName+"/";

      tree += F("</a></td>");

      tree += F("<td class=\"dCol\" data-value=\"");
      tree += entryTime;
      tree += F("\">");
      tree += entryTime;
      tree += F("</td>");

      tree += F("<td class=\"dCol\" data-value=\"0\">-</td>");
      
      if (!webfileReadonly) {
        tree += F("<td class=\"dCol\" data-value=\"0\">");
        tree += F("<button class='buttons' onclick=\"location.href='/deleteConfirm?folder=");
        tree += entryPath;
        tree += F("';\">Delete</button></td>");
      }
      
      tree += F("</tr>\n");
     
    } else {
      tree += F("<tr>");
      tree += F("<td data-value=\"");
      tree += entryName;
      
      tree += F("\"><a class=\"icon file\" draggable=\"true\" href=\"");
      if (shortUrl)
        tree += entryName;
      else
        tree += entryPath;
      tree +=  F("\">");

      tree += F("&nbsp;&nbsp;&nbsp;&nbsp;");

      tree += F("</a><a href=\"");
      if (shortUrl)
        tree += entryName;
      else
        tree += entryPath;
      tree += F("\">");
      tree += entryName;

      tree += F("</a></td>");

      tree += F("<td class=\"dCol\" data-value=\"");
      tree += entryTime;
      tree += F("\">");
      tree += entryTime;
      tree += F("</td>");

      tree += F("<td class=\"dCol\" data-value=\")");
      tree += file_size(entry.size());
      tree += F("\">");
      tree += file_size(entry.size());
      tree += F("</td>");
      
      if (!webfileReadonly) {
        tree += F("<td class=\"dCol\" data-value=\"0\">");
        tree += F("<button class='buttons' onclick=\"location.href='/deleteConfirm?file=");
        tree += entryPath;
        tree += F("';\">Delete</button></td>");
      }

      tree += F("</tr>\n");
    }

  return tree;
}

void handleConsole(AsyncWebServerRequest *request) {
  if (!handleLogin(request))
    return;

  String entryName = "";
  String entryPath = "";
  String entryTime = "";
  String tree = "";

  if (!webfileDisabled) {
    entryName = getLastElement(WifiWebAppData);
    entryPath = WifiWebAppData;
    entryTime = "";

      tree += F("<tr>");
      tree += F("<td data-value=\"");
      tree += entryName+"/";
      
      tree += F("\"><a class=\"icon dir\" href=\"");
      tree += entryPath+"/";
      tree += F("\">");

      tree += F("&nbsp;&nbsp;&nbsp;&nbsp;");

      tree += F("</a><a href=\"");
      tree += entryPath+"/";
      tree += F("\">");
      tree += entryName+"/";

      tree += F("</a></td>");

      tree += F("<td class=\"dCol\" data-value=\"");
      tree += entryTime;
      tree += F("\">");
      tree += entryTime;
      tree += F("</td>");

      tree += F("<td class=\"dCol\" data-value=\"0\">-</td>");

      tree += F("</tr>\n");
  }

    entryName = "screen.bmp";
    entryPath = "/"+entryName;
    entryTime = getLocalTimeString();
    
      if (entryPath == "/screen.bmp") {
      //MG.lcd.Save();

      //tree += F("<img id=\"screen-shot\" src=\"");
      tree += F("<img id=\"screen-shot\" src=\"data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7\" data-src=\"");
      tree += entryPath;
      tree += F("\" alt=\"\"  width=\"");
      tree += 320;
      tree += F("\" height=\"");
      tree += MG.lcd.GetBitmapHeight();
      tree += F("\">");

      tree += F("<tr>");
      tree += F("<td data-value=\"");
      tree += entryPath;
      
      tree += F("\"><a class=\"icon file\" draggable=\"true\" href=\"");
      tree += entryPath;
      tree +=  F("\">");

      tree += F("&nbsp;&nbsp;&nbsp;&nbsp;");

      tree += F("</a><a href=\"");
      tree += entryPath;
      tree += F("\">");
      tree += entryName;

      tree += F("</a></td>");
      
      tree += F("<td class=\"dCol\" data-value=\"");
      tree += entryTime;
      tree += F("\">");
      tree += entryTime;
      tree += F("</td>");

      tree += F("<td class=\"dCol\" data-value=\")");
      tree += file_size(MG.lcd.GetBitmapSize());
      tree += F("\">");
      tree += file_size(MG.lcd.GetBitmapSize());
      tree += F("</td>");

      tree += F("<td class=\"dCol\" data-value=\"0\">");
      tree += F("<button class='buttons' onclick=\"updateImages()");
      tree += F(";\">Refresh</button></td>");

      tree += F("</tr>\n");
      }

  String webpage = "";

  webpage += header;

  webpage.replace("Web FileBrowser", "Remote Console");
  webpage += F("<h1 id=\"header\">Remote Console");
  webpage += F("</h1>");

  webpage += script;

  webpage += F("<table>\n");
  webpage += F("<thead>\n");
  webpage += F("<tr class=\"header\" id=\"theader\">");
  webpage += F("<th onclick=\"sortTable(0);\">Name</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(1);\">Last modified</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(2);\">Size</th>");
  webpage += F("<th>");

  webpage += htmlButtonDirect("Restart", "restart",  "class='buttons' style=\"float: right;\"");

  webpage += F("</th>");
  webpage += F("</tr>\n");
  webpage += F("</thead>\n");

  webpage += F("<tbody id=\"tbody\">\n");

  webpage += tree;
  tree = "";

  webpage += F("</tbody>\n");
  webpage += F("</table>\n");
  webpage += F("<hr>");

  webpage += F("<table>\n");

  webpage += F("<tr>\n");

  webpage += F("<td>\n");
  webpage += htmlButtonPress("Menu", "Menu");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += htmlButtonPress("Volume", "Volume");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += htmlButtonPress("Select", "Select");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += htmlButtonPress("Start", "Start");
  webpage += F("</td>\n");

  webpage += F("</tr>\n");


  webpage += F("<tr>\n");

  webpage += F("<td>\n");
  webpage += F("&nbsp;&nbsp;&nbsp;&nbsp;");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += F("&nbsp;&nbsp;&nbsp;&nbsp;");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += F("&nbsp;&nbsp;&nbsp;&nbsp;");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += F("&nbsp;&nbsp;&nbsp;&nbsp;");
  webpage += F("</td>\n");

  webpage += F("</tr>\n");

  webpage += F("<tr>\n");

  webpage += F("<td>\n");
  webpage += F("&nbsp;&nbsp;&nbsp;&nbsp;");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += htmlButtonPress("&uarr;", "Up");
  webpage += F("</td>");

  webpage += F("<td>\n");
  webpage += F("&nbsp;&nbsp;&nbsp;&nbsp;");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += htmlButtonPress("A", "A",  "class='buttons' style=\"float: right;\"");
  webpage += F("</td>\n");

  webpage += F("</tr>\n");


  webpage += F("<tr>\n");

  webpage += F("<td>\n");
  webpage += htmlButtonPress("&larr;", "Left",  "class='buttons' style=\"float: right;\"");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += htmlButtonPress("&rarr;", "Right", "class='buttons' style=\"float: right;\"");
  webpage += F("</td>\n");

  webpage += F("<td class=\"ledCol\">\n");
  if (MG.led.isOn())
    webpage += htmlButtonPress("LED", "LED", "class=\"ledbtn ledbtnon\"");
  else
    webpage += htmlButtonPress("LED", "LED", "class=\"ledbtn ledbtnoff\"");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += htmlButtonPress("B", "B");
  webpage += F("</td>\n");

  webpage += F("</tr>\n");


  webpage += F("<tr>\n");

  webpage += F("<td>\n");
  webpage += F("&nbsp;&nbsp;&nbsp;&nbsp;");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += htmlButtonPress("&darr;", "Down");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += F("&nbsp;&nbsp;&nbsp;&nbsp;");
  webpage += F("</td>\n");

  webpage += F("<td>\n");
  webpage += F("&nbsp;&nbsp;&nbsp;&nbsp;");
  webpage += F("</td>\n");

  webpage += F("</tr>\n");

  
  webpage += F("</table>\n");

  webpage += footer_script;

  webpage += F("<script>\n");
  webpage += F("function add_time(link)\n");
  webpage += F("{\n");
  webpage += F("  location.href=link.href + '&time=' + new Date().getTime();\n");
  webpage += F("  return false;\n");
  webpage += F("}\n");
  webpage += F("</script>\n");

  webpage += footer;

  webpage.replace("WiFi Web - ", "<a href=\"/sysinfo\">System Information</a> - <a href=\"/syssetup?page=7\" onclick=\"return add_time(this);\">Setup</a> - WiFi Web - ");

  request->send(200, "text/html", webpage);
}

void handleRoot(AsyncWebServerRequest *request) {
  if (!handleLogin(request))
    return;

  String directory = urldecode(request->url());
  uploadPath = directory;

  String entryName = "";
  String entryPath = "";
  String tree = "";
  char* c_tree = NULL;
  size_t c_len = 0;
  bool emptyFolder = true;
  bool bigFolder = false;
  bool shortUrl = false;
  MemInfo memnow;
/*
  Serial.print("path:");
  Serial.println(directory);
*/
  String parentdir = removeLastElement(directory);
  String lastElement = getLastElement(directory);
  if (lastElement == "") {
    shortUrl = true;
    if (parentdir.length() > 1)
      parentdir = parentdir.substring(0, parentdir.length() - 1);
    directory = parentdir;
    uploadPath = directory;
    parentdir = removeLastElement(directory);
    lastElement = getLastElement(directory);
  }

  if (parentdir.length() > 1)
    parentdir = parentdir.substring(0, parentdir.length() - 1);
/*
  Serial.print("parentdir:");
  Serial.println(parentdir);

  Serial.print("lastElement:");
  Serial.println(lastElement);
*/  

  if (directory == "/") {
    handleConsole(request);
    return;
  }  else if (directory == "/screen.bmp") {
    handleScreenImg(request);
    return;
  }

  if (webfileDisabled) {
    handleNotFound(request);
    return;
  }

  File dir;

    dir = SD.open(directory);

    FillMemInfo(&memnow);
  
#ifdef ESP32_WITH_PSRAM
    Serial.print("MaxAlloc: ");
    Serial.print((int32_t)memnow.Psram.MaxAlloc);
    Serial.print(" Free: ");
    Serial.println((int32_t)memnow.Psram.Free);
#endif
    while (true) {
      File entry =  dir.openNextFile();

      if (!entry) {
        // no more files
        // return to the first file in the directory
        dir.rewindDirectory();
        break;
      }

      String treeEntry = htmlDirEntry(entry, shortUrl);
      c_len += treeEntry.length();

      emptyFolder = false;

      entry.close();

      if (c_len >= 24576) {
        bigFolder = true;
      }
    }

    /* Inform the watchdog this task is alive */
    if (bigFolder) {
#ifdef ESP32
      esp_task_wdt_reset();
#elif defined(ESP8266)
      ESP.wdtFeed();
#else
      wdt_reset();
#endif
      bigFolder = false;
    }

    FillMemInfo(&memnow);

    Serial.print("Tree: ");
    Serial.print(c_len);
    Serial.print(" MaxAlloc: ");
#ifdef ESP32_WITH_PSRAM
    Serial.print((int32_t)memnow.Psram.MaxAlloc);
    Serial.print(" Free: ");
    Serial.println((int32_t)memnow.Psram.Free);
#else
    Serial.println(bm_max_alloc());
#endif

  String webpage = "";

  webpage += header;

    webpage += F("<h1 id=\"header\">Index of ");
    webpage += directory;
    webpage += F("</h1>");

    webpage += F("<div id=\"parentDirLinkBox\" style=\"display:block;\">");
    
    webpage += F("<a id=\"parentDirLink\" class=\"icon up\" href=\"");
    webpage += parentdir;
    if (parentdir != "/") {
      webpage += "/";
    }
    webpage += F("\">");
    webpage += F("&nbsp;&nbsp;&nbsp;&nbsp;");
    webpage += F("</a>");

    webpage += F("<a id=\"parentDirLink\" href=\"");
    webpage += parentdir;
    if (parentdir != "/") {
      webpage += "/";
    }
    webpage += F("\">");
    webpage += F("<span id=\"parentDirText\">[Parent directory]</span>");
    webpage += F("</a>");
    
    webpage += F("</div>");

  if ((tree == "") && !c_len) {
    String dlPath = directory;

    if (SD.exists(dlPath)) {
      File entry = SD.open(dlPath);
      if (!entry.isDirectory()) {
        Serial.print("path: ");
        Serial.println(dlPath);

        Serial.print("parentdir: ");
        Serial.println(parentdir);

        Serial.print("lastElement: ");
        Serial.println(lastElement);

        if (parentdir == "/") {
          handleNotFound(request);
        } else if (!handleFileRead(request, dlPath)) {
          handleNotFound(request);
        }

        return;
      }
    }
    else {
      handleNotFound(request);
      return;
    }
  }

  webpage += script;

  webpage += F("<table>\n");
  webpage += F("<thead>\n");
  webpage += F("<tr class=\"header\" id=\"theader\">");
  webpage += F("<th onclick=\"sortTable(0);\">Name</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(1);\">Last modified</th>");
  webpage += F("<th class=\"dCol\" onclick=\"sortTable(2);\">Size</th>");
  webpage += F("<th>");
  webpage += F("</th>");
  webpage += F("</tr>\n");
  webpage += F("</thead>\n");

  webpage += F("<tbody id=\"tbody\">\n");

  webpage += tree;
  tree = "";

  String webhead = "";
  if (c_len) {
    webhead = webpage;
    webpage = "";
  }

  webpage += F("</tbody>\n");
  webpage += F("</table>\n");
  webpage += F("<hr>");

    if (!webfileReadonly) {
      webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
      webpage += F("<input class='buttons' type='file' name='fupload' id = 'fupload' value=''>");
      webpage += F("<button class='buttons' type='submit'>Upload</button></form><br>");

      webpage += F("<FORM action='/mkdir' method='post' enctype='multipart/form-data'>");
      webpage += F("<input type='hidden' id='path' name='path' value='");
      webpage += uploadPath;
      webpage += F("'>");
      webpage += F("<input class='buttons' name='dirName' id ='dirName' value='NewFolder'>");
      webpage += F("<button class='buttons' type='submit'>MkDir</button></form>");
    }

    webpage += F("<br>");

  webpage += footer_script;

  webpage += footer;

  if (c_len) {
    size_t w_len = webhead.length()+webpage.length()+1;

    if (w_len > bm_max_alloc()) {
      w_len = 0;
      c_len = 0;
    } else if (w_len+c_len > bm_max_alloc()) {
      c_len = bm_max_alloc()-w_len;
    }

    w_len = w_len+c_len;

    if (w_len) {
      c_tree = (char*)bm_malloc(w_len);
    }

    if (!c_tree) {
      String webfoot = webpage;
      webpage = webhead;
      webpage += F("<p>(Folder too big)</p>\n");
      webpage += webfoot;
    }
  }

  if (c_tree) {
    size_t treeResvd = webhead.length()+c_len;
    strcpy(c_tree, webhead.c_str());
    c_len = webhead.length();
    webhead = "";

    Serial.print("Head: ");
    Serial.println(c_len);

    while (true) {
      File entry =  dir.openNextFile();

      if (!entry) {
        // no more files
        // return to the first file in the directory
        dir.rewindDirectory();
        break;
      }

      String treeEntry = htmlDirEntry(entry, shortUrl);
      strcat(c_tree, treeEntry.c_str());
      c_len += treeEntry.length();

      emptyFolder = false;

      entry.close();

      if (c_len >= treeResvd) {
        bigFolder = true;
        // return to the first file in the directory
        dir.rewindDirectory();
        break;
      }
    }

    strcat(c_tree, webpage.c_str());
    c_len += webpage.length();
    webpage = "";

    Serial.print("Page: ");
    Serial.println(c_len);

    streamBuf(request, (uint8_t*)c_tree, c_len, "text/html");

    bm_free(c_tree);

  } else {
    request->send(200, "text/html", webpage);
  }
}

void doConfirmButton(AsyncWebServerRequest *request, String press, String ask) {
  if (!handleLogin(request))
    return;

  String webpage = "";
  webpage += header;

      webpage += F("<hr>Do you want to ");
      webpage += ask;
      webpage += F("?<br>");
      webpage += F("<br><br><button class='buttons' onclick=\"location.href='/button?press=");
      webpage += press;
      webpage += F("&sure=yes");
      webpage += F("';\">Yes</button>");

  webpage += F("<button class='buttons' onclick='window.history.back();'>No</button>");
  webpage += footer;
  request->send(200, "text/html", webpage);
}

void doButton(AsyncWebServerRequest *request) {
  if (!handleLogin(request))
    return;

  String webpage = "";
  webpage += header;
  String press = "";
  String sure = "";
  bool doRestart = false;

  for (uint8_t i = 0; i < request->args(); i++) {
    if (request->argName(i) == "sure") {
      //Serial.printf("sure: %s\n", request->arg(i));
      sure =  request->arg(i);
    }
    if (request->argName(i) == "press") {
      //Serial.printf("Press: %s\n", request->arg(i));
      press = request->arg(i);
    }

  }
  
  if (press != "") {
    webpage += F("<hr>Pressing Button: <br>");

    webpage += press;

    if (sure == "yes") {
        if (press == "restart") {
            doRestart = true;
        }
    }

    else if (press == "restart") {
        doConfirmButton(request, press, "restart device");
        return;
    } else if (press == "Menu")
        MG.BtnMenu.virtualPin = 1;
    else if (press == "Volume")
        MG.BtnVolume.virtualPin = 1;
    else if (press == "Select")
        MG.BtnSelect.virtualPin = 1;
    else if (press == "Start")
        MG.BtnStart.virtualPin = 1;
        
    else if (press == "Up")
        MG.JOY_Y.virtualPin = 2;
    else if (press == "Down")
        MG.JOY_Y.virtualPin = 1;
    else if (press == "Left")
        MG.JOY_X.virtualPin = 2;
    else if (press == "Right")
        MG.JOY_X.virtualPin = 1;
        
    else if (press == "A")
        MG.BtnA.virtualPin = 1;
    else if (press == "B")
        MG.BtnB.virtualPin = 1;

    else if (press == "LED")
        MG.led.set(!MG.led.get());
        
    //Serial.printf("Pressing Button: %s\n", press);
    webpage += F("<br>OK<br>");
  } else {
    webpage += F("<br>press or Name empty!");
  }

  webpage += F("<br><button class='buttons' onclick=\"location.href='/';\">OK</button>");
  webpage += goback_script;
  webpage += footer;
  request->send(200, "text/html", webpage);

  if (doRestart) {
    SaveWifiWebMode();
    ESP.restart();
  }
}

bool LoadWifiWebMode() {
  bool ret = false;

      String path = "/WIFIMODE.TXT";
      Serial.print("Reading file: ");
      Serial.println(path);

      File wifiModeFile = SD.open(path);
      if (wifiModeFile) {
        Serial.println("Read from file: ");
        while (wifiModeFile.available()) {
          String wifiModeValue = wifiModeFile.readStringUntil('\n');
          wifiModeValue.replace("\r", "");
          WifiWebMode = wifiModeValue.toInt();

          if (wifiModeFile.available()) {
            wifiModeValue = wifiModeFile.readStringUntil('\n');
            wifiModeValue.replace("\r", "");
            webfileReadonly = wifiModeValue.toInt();
          }

          if (wifiModeFile.available()) {
            wifiModeValue = wifiModeFile.readStringUntil('\n');
            wifiModeValue.replace("\r", "");
            webfileDisabled = wifiModeValue.toInt();
          }

          ret = true;
          break;
        }
        wifiModeFile.close();
      }

  return ret;
}

bool SaveWifiIPData(String SSID) {
  bool ret = false;
  String path = "/WIFIIP-";

  path += SSID;
  path += ".TXT";

  if (NewWifiIPMode == 1) {
      Serial.print("Writing file: ");
      Serial.println(path);

      File wifiIPFile = SD.open(path, FILE_WRITE);
      if (!wifiIPFile) {
          Serial.println("Failed to open file for writing");
      } else {
          wifiIPFile.println(NewWifiIP.toString());
          wifiIPFile.println(NewWifiMask.toString());
          wifiIPFile.println(NewWifiGW.toString());
          wifiIPFile.println(NewWifiDNS1.toString());
          wifiIPFile.println(NewWifiDNS2.toString());

          ret = true;

          wifiIPFile.close();
      }
  } else {
      if (SD.exists(path)) {
          SD.remove(path);
      }
  }

  return ret;
}

bool SaveWifiWebMode() {
  bool ret = false;

  GetNewWifiWebMode();

      String path = "/WIFIMODE.TXT";
      Serial.print("Writing file: ");
      Serial.println(path);

        File wifiModeFile = SD.open(path, FILE_WRITE);
        if (!wifiModeFile) {
          Serial.println("Failed to open file for writing");
        } else {
          String wifiModeValue = String(NewWifiWebMode,DEC);
          wifiModeFile.println(wifiModeValue);

          wifiModeValue = String(webfileReadonly,DEC);
          wifiModeFile.println(wifiModeValue);

          wifiModeValue = String(webfileDisabled,DEC);
          wifiModeFile.println(wifiModeValue);

          ret = true;

          wifiModeFile.close();
        }

  if (NewWifiWebMode == 1) {
      path = "/WIFIAP.TXT";
      Serial.print("Writing file: ");
      Serial.println(path);

        File wifiAPFile = SD.open(path, FILE_WRITE);
        if (!wifiAPFile) {
          Serial.println("Failed to open file for writing");
        } else {
          wifiAPFile.println(NewWifiAPSSID);
          wifiAPFile.println(NewWifiAPPSK);
          wifiAPFile.println(NewWifiAPIP.toString());
          wifiAPFile.println(NewWifiAPMask.toString());
          wifiAPFile.println(NewWifiAPGW.toString());
          wifiAPFile.println(NewWifiAPHost);
          wifiAPFile.close();
        }
  } else if (NewWifiWebMode == 2) {
      int wifiIndex = 0;
      int newIndex = WifiCount;
      path = "/WIFI.TXT";
      Serial.print("Writing file: ");
      Serial.println(path);

        File wifiFile = SD.open(path, FILE_WRITE);
        if (!wifiFile) {
          Serial.println("Failed to open file for writing");
        } else {
/*
          wifiFile.println(NewWifiSSID);
          wifiFile.println(NewWifiPSK);
*/
          while (wifiIndex < WifiCount) {
            if (WifiList[wifiIndex].SSID == NewWifiSSID) {
              WifiList[wifiIndex].PSK = NewWifiPSK;
              newIndex = wifiIndex;
            }
            wifiFile.println(WifiList[wifiIndex].SSID);
            wifiFile.println(WifiList[wifiIndex].PSK);
            wifiIndex += 1;
          }

          if (newIndex == WifiCount) {
            wifiFile.println(NewWifiSSID);
            wifiFile.println(NewWifiPSK);
          }

          wifiFile.close();
        }

      SaveWifiIPData(NewWifiSSID);
  }

      path = "/WIFIWEB.TXT";
      Serial.print("Writing file: ");
      Serial.println(path);

        File wifiWebFile = SD.open(path, FILE_WRITE);
        if (!wifiWebFile) {
          Serial.println("Failed to open file for writing");
        } else {

          wifiWebFile.println(WifiWebUser);
          wifiWebFile.println(WifiWebPass);
          wifiWebFile.close();
        }

      path = "/WIFINTP.TXT";
      Serial.print("Writing file: ");
      Serial.println(path);

        File wifiNtpFile = SD.open(path, FILE_WRITE);
        if (!wifiNtpFile) {
          Serial.println("Failed to open file for writing");
        } else {

          wifiNtpFile.println(ntpServer1);
          if ((ntpServer2 != "") && (ntpServer3 != "")) {
            wifiNtpFile.println(ntpServer2);
            wifiNtpFile.println(ntpServer3);
          }
          wifiNtpFile.println(gmtOffset_sec);
          wifiNtpFile.println(daylightOffset_sec);
          wifiNtpFile.close();
        }

  return ret;
}

void disableWifi() {
  if (dnsStarted) {
    dnsServer.stop();
    dnsStarted = false;
  }
  if (WifiWebMode) {
    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    WifiWebMode = 0;
    String path = "/WIFIMODE.TXT";
    if (SD.exists(path)) {
      SD.remove(path);
    }
  }
}

bool sd_init() {
  Serial.print("WiFiWeb: Check heap before SD: ");
  if (bm_check_integrity(true)) {
    Serial.println("OK");
  } else {
    Serial.println("Failed!");
  }

  MG.lcd.clear();

  Serial.print("WiFiWeb: Check heap before MG.lcd.setCursor: ");
  if (bm_check_integrity(true)) {
    Serial.println("OK");
  } else {
    Serial.println("Failed!");
  }

  MG.lcd.setCursor(0, 0);
  MG.lcd.setTextWrap(false);

  Serial.println("");

  Serial.print("Initializing SD card...");
  MG.lcd.println("Initializing SD card...");

#ifdef ESP32
  if (!SD.begin()) {
#else
  if (!SD.begin(SD_CS_PIN)) {
#endif
    MG.lcd.println("Card Mount Failed");
    Serial.println("Card Mount Failed");
    return false;
  }

  FillMemInfo(&memboot);

#ifdef ESP32
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    MG.lcd.println("No SD card attached");
    Serial.println("No SD card attached");
    return false;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    MG.lcd.println("MMC");
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    MG.lcd.println("SDSC");
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    MG.lcd.println("SDHC");
    Serial.println("SDHC");
  } else {
    MG.lcd.println("UNKNOWN");
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  MG.lcd.printf("SD Card Size: %lluMB\n", cardSize);
#endif //ESP32

  MG.lcd.println(" ");
  Serial.println();

  return true;
}

void printLocalTime()
{
  struct tm timeinfo;
  char buffer[40];

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  strftime(buffer, 40, "%A, %B %d %Y %H:%M:%S", &timeinfo);
  Serial.println(buffer);
  GO.lcd.println(buffer);
}

void wifiweb_info() {
  GO.lcd.println();
  Serial.println();

  if (WifiWebMode == 1) {
      GO.lcd.println("WiFi mode: AP");
      Serial.println("WiFi mode: AP");
      GO.lcd.println();
      Serial.println();

      GO.lcd.print("AP SSID: "); GO.lcd.println(WifiAPSSID);
      GO.lcd.print("AP PSK: "); GO.lcd.println(WifiAPPSK);

      IPAddress myIP = WiFi.softAPIP();
      Serial.print("AP IP address: "); Serial.println(myIP);
      GO.lcd.print("AP IP address: "); GO.lcd.println(myIP);
  }
  else if (WifiWebMode == 2) {
      GO.lcd.println("WiFi mode: Client");
      Serial.println("WiFi mode: Client");
      GO.lcd.println();
      Serial.println();

      Serial.print("SSID: ");
      Serial.println(WifiSSID);
      Serial.print("PSK: ");
      Serial.println(WifiPSK);
      GO.lcd.print("SSID: '");
      GO.lcd.print(WifiSSID);
      GO.lcd.println("'");
      GO.lcd.print("PSK: '");
      GO.lcd.print(WifiPSK);
      GO.lcd.println("'");
      Serial.println();

      Serial.println();
      IPAddress myIP = WiFi.localIP();
      Serial.print("IP address: ");
      Serial.println(myIP);
      GO.lcd.print("IP address: ");
      GO.lcd.println(myIP);
     
      printLocalTime();

  } else {
      GO.lcd.println("WiFi mode: disabled");
      Serial.println("WiFi mode: disabled");
      GO.lcd.println();
      Serial.println();

      return;
  }

  GO.lcd.println();
  Serial.println();

  GO.lcd.print("Web Username: "); GO.lcd.println(WifiWebUser);
  GO.lcd.print("Web Password: "); GO.lcd.println(WifiWebPass);

  GO.lcd.println();
  Serial.println();
}

bool wifi_init(bool interactive) {
  GO.update();
  
  Serial.println("WiFiWeb: Initialization begin");

  GO.lcd.println(" ");
  Serial.println();

  GO.lcd.println("-Wifi Mode-");
  if (!LoadWifiWebMode() && interactive) {
    GO.lcd.println("-> Press A for Access Point mode");
    GO.lcd.println("-> Press B to connect to a Wifi");
    GO.lcd.println("-> Press Menu to disable Wifi");
    GO.lcd.println(" ");
    Serial.println();
    while (true) {
      if (GO.BtnA.isPressed()) {
        Serial.println("WiFiWeb: Btn A pressed");

        WifiWebMode = 1;
        break;
      }
      if (GO.BtnB.isPressed()) {
        Serial.println("WiFiWeb: Btn B pressed");
      
        WifiWebMode = 2;
        break;
      }
      if (GO.BtnMenu.isPressed() == 1) {
        Serial.println("WiFiWeb: Btn Menu pressed");

        WifiWebMode = 0;
        break;
      }
      GO.update();
    }
  }

      String path = "/WIFINTP.TXT";
      Serial.print("Reading file: ");
      Serial.println(path);
      GO.lcd.print("Reading file: ");
      GO.lcd.println(path);
      File wifiNtpFile = SD.open(path);
      if (!wifiNtpFile) {
        Serial.println("Failed to open file for reading");
        GO.lcd.println("Failed to open file for reading");

      } else {
        Serial.println("Read from file: ");
        GO.lcd.println("Read from file: ");
        while (wifiNtpFile.available()) {
          ntpServer1 = wifiNtpFile.readStringUntil('\n');
          ntpServer1.replace("\r", "");
          String gmtOffsetValue = wifiNtpFile.readStringUntil('\n');
          gmtOffsetValue.replace("\r", "");
          String dstOffsetValue = wifiNtpFile.readStringUntil('\n');
          dstOffsetValue.replace("\r", "");

          if (wifiNtpFile.available()) {
            ntpServer2 = gmtOffsetValue;
            ntpServer3 = dstOffsetValue;
            gmtOffsetValue = wifiNtpFile.readStringUntil('\n');
            gmtOffsetValue.replace("\r", "");
            dstOffsetValue = wifiNtpFile.readStringUntil('\n');
            dstOffsetValue.replace("\r", "");
          } else {
            ntpServer2 = "";
            ntpServer3 = "";
          }

          gmtOffset_sec = gmtOffsetValue.toInt();
          daylightOffset_sec = dstOffsetValue.toInt();
          break;
        }
        wifiNtpFile.close();
      }

  if (WifiWebMode == 1) {
      WifiAPSSID = ssid;
      WifiAPPSK = password;

      if (!GetWifiAPData(true)) {
        WifiWebMode = 0;
        return false;
      }

      WiFi.mode(WIFI_AP);

      // You can remove the password parameter if you want the AP to be open.
      if (!WiFi.softAP(WifiAPSSID.c_str(), WifiAPPSK.c_str())) {
        WifiWebMode = 0;
        return false;
      }

      WiFi.softAPConfig(WifiAPIP, WifiAPGW, WifiAPMask);
#ifdef ESP32
      WiFi.softAPsetHostname(WifiAPHost.c_str());
#else
      WiFi.hostname(WifiAPHost.c_str());
#endif

      IPAddress myIP = WiFi.softAPIP();

      GO.lcd.println("AP started");
      GO.lcd.print("AP SSID: "); GO.lcd.println(WifiAPSSID);
      GO.lcd.print("AP PSK: "); GO.lcd.println(WifiAPPSK);

      Serial.print("AP IP address: "); Serial.println(myIP);
      GO.lcd.print("AP IP address: "); GO.lcd.println(myIP);

      GO.lcd.println(" ");
      Serial.println();

      String FQDN = WifiAPHost + ".local";

      dnsStarted = dnsServer.start(DNS_PORT, FQDN, myIP);
      if (dnsStarted) {
        Serial.println("DNS responder started");
        GO.lcd.println("DNS responder started");
        Serial.print("You can connect via http://");
        Serial.println(FQDN);
        GO.lcd.print("You can connect via http://");
        GO.lcd.println(FQDN);
      }

  } else if (WifiWebMode == 2) {
      int wifiIndex = 0;
      WifiSSID = "";
      WifiPSK = "";

      if (!GetWifiData()) {
        WifiWebMode = 0;
        return false;
      }

      Serial.print("SSID: ");
      Serial.println(WifiSSID);
      Serial.print("PSK: ");
      Serial.println(WifiPSK);
      GO.lcd.print("SSID: '");
      GO.lcd.print(WifiSSID);
      GO.lcd.println("'");
      GO.lcd.print("PSK: '");
      GO.lcd.print(WifiPSK);
      GO.lcd.println("'");
      Serial.println();

      WiFi.mode(WIFI_AP_STA);

      //delete old wifi Credentials
      WiFi.disconnect();
      WiFi.mode(WIFI_STA);

      WiFi.begin(WifiSSID.c_str(), WifiPSK.c_str());
      Serial.print("Connecting Wifi");
      GO.lcd.print("Connecting Wifi");
      uint32_t ms = 7500;
      uint32_t start = millis();
      while (WiFi.status() != WL_CONNECTED) {
        if ((millis()-start) > ms) {
            wifiIndex++;
            if (wifiIndex >= WifiCount) {
                wifiIndex = 0;
                ms *= 2;
                if (ms > 60000) {
                  ms = 7500;
                }
            }
            if (wifiIndex < WifiCount) {
              if ((WifiSSID != WifiList[wifiIndex].SSID) || 
                  (WifiPSK  != WifiList[wifiIndex].PSK )) {
                GO.lcd.println(" ");
                Serial.println();
                WifiSSID = WifiList[wifiIndex].SSID;
                WifiPSK = WifiList[wifiIndex].PSK;
                WiFi.disconnect();
                Serial.print("SSID: ");
                Serial.println(WifiSSID);
                WiFi.begin(WifiSSID.c_str(), WifiPSK.c_str());
              }
            }
            start = millis();
        }

        delay(250);
        GO.update();
        if (GO.BtnMenu.isPressed() == 1) {
            break;
        } else if (GO.BtnSelect.isPressed() == 1) {
            disableWifi();
            break;
        }

        delay(250);
        GO.update();
        if (GO.BtnMenu.isPressed() == 1) {
            break;
        } else if (GO.BtnSelect.isPressed() == 1) {
            disableWifi();
            break;
        }

        Serial.print(".");
        GO.lcd.print(".");
      }

      GO.lcd.println(" ");
      Serial.println();

      if (WiFi.status() != WL_CONNECTED) {
        WifiWebMode = 0;
        return false;
      }

      WifiIPMode = 2;
      if (GetWifiIPData(WifiSSID)) {
        WifiIPMode = 1;
        WiFi.config(WifiIP, WifiGW, WifiMask, WifiDNS1, WifiDNS2);
      }
#ifdef ESP32
      WiFi.setHostname(WifiHost.c_str());
#else
      WiFi.hostname(WifiHost.c_str());
#endif

      WifiIP = WiFi.localIP();
#ifdef ESP32
      WifiMask = calculateCIDRSubnet(WiFi.subnetCIDR());
#else
      WifiMask = WiFi.subnetMask();
#endif
      WifiGW = WiFi.gatewayIP();
      WifiDNS1 = WiFi.dnsIP(0);
      WifiDNS2 = WiFi.dnsIP(1);

      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      GO.lcd.print("IP address: ");
      GO.lcd.println(WiFi.localIP());

      GO.lcd.println(" ");
      Serial.println();

      String FQDN = WifiHost + ".local";

      if (MDNS.begin(WifiHost.c_str())) {
        Serial.println("MDNS responder started");
        GO.lcd.println("MDNS responder started");
        Serial.print("You can connect via http://");
        Serial.println(FQDN);
        GO.lcd.print("You can connect via http://");
        GO.lcd.println(FQDN);
      }

      //init and get the time
      if (ntpServer2 != "") {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1.c_str(), ntpServer2.c_str(), ntpServer3.c_str());
      } else {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1.c_str());
      }
      printLocalTime();

  } else {
      return false;
  }

  GO.lcd.println(" ");
  Serial.println();

      path = "/WIFIWEB.TXT";
      Serial.print("Reading file: ");
      Serial.println(path);
      GO.lcd.print("Reading file: ");
      GO.lcd.println(path);
      File wifiWebFile = SD.open(path);
      if (!wifiWebFile) {
        Serial.println("Failed to open file for reading");
        GO.lcd.println("Failed to open file for reading");

        File wifiWebFile = SD.open(path, FILE_WRITE);
        if (!wifiWebFile) {
          Serial.println("Failed to open file for writing");
          GO.lcd.println("Failed to open file for writing");
        } else {
          WifiWebPass+="-";
          WifiWebPass+=String(random(65535),HEX);

          wifiWebFile.println(WifiWebUser);
          wifiWebFile.println(WifiWebPass);
          wifiWebFile.close();
        }
      } else {
        Serial.println("Read from file: ");
        GO.lcd.println("Read from file: ");
        while (wifiWebFile.available()) {
          WifiWebUser = wifiWebFile.readStringUntil('\n');
          WifiWebUser.replace("\r", "");
          WifiWebPass = wifiWebFile.readStringUntil('\n');
          WifiWebPass.replace("\r", "");
          break;
        }
        wifiWebFile.close();
      }

  GO.lcd.println(" ");
  Serial.println();

  return true;
}

AsyncWebServer *wifiweb_init(bool interactive) {
  Serial.print("WiFiWeb: Check heap before WiFi: ");
  if (bm_check_integrity(true)) {
    Serial.println("OK");
  } else {
    Serial.println("Failed!");
  }

  if (!wifi_init(interactive))
    return NULL;

  if (wifiweb)
    return wifiweb;

  server.on("/", [](AsyncWebServerRequest *request) {
    handleRoot(request);
  });

  server.on("/sysinfo", HTTP_GET, [](AsyncWebServerRequest *request){
    handleSysInfo(request);
  });

  server.on("/syssetup", HTTP_GET, [](AsyncWebServerRequest *request){
    handleSysSetup(request);
  });

  server.on("/syssetup", HTTP_POST, [](AsyncWebServerRequest *request){
    doSaveSetup(request);
  });

  server.onNotFound(handleRoot);

  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });

  server.on("/fupload",  HTTP_POST, [](AsyncWebServerRequest *request) {
    doFUpload(request);
  }, handleFileUpload);

  server.on("/deleteConfirm", deleteConfirm);
  server.on("/doDelete", doDelete);
  server.on("/mkdir", doMkdir);

  server.on("/screen.bmp", handleScreenImg);

  server.on("/button", doButton);

  server.begin();
  Serial.println("HTTP server started");
  GO.lcd.println("HTTP server started");

  GO.lcd.print("Web Username: "); GO.lcd.println(WifiWebUser);
  GO.lcd.print("Web Password: "); GO.lcd.println(WifiWebPass);

  GO.lcd.println();
  Serial.println();

  Serial.println("WiFiWeb: Initialization done.");
  GO.lcd.println("WiFiWeb: Initializing done.");
  
  return &server;
}

bool setupWiFiWeb(String directory) {
    SetWifiWebAppData(directory);

    if (!wifiweb)  {
        wifiweb = wifiweb_init(false);
    } else {
        wifiweb_info();
    }

    return (wifiweb && WifiWebMode);
}

bool showWiFiWeb() {
    MG.lcd.clear();

    if (wifiweb && WifiWebMode)  {
        wifiweb_info();

        GO.lcd.println(" ");
        GO.lcd.println("-> Press Select to disable Wifi");
    } else {
        wifiweb = wifiweb_init();

        GO.lcd.println(" ");
    }

    GO.lcd.println("-> Press Menu to go back");
    GO.lcd.println(" ");
    
    while (true) {
        MG.update();

        if (MG.BtnMenu.isPressed() == 1) {
            break;
        } else if (MG.BtnSelect.isPressed() == 1) {
            disableWifi();
            break;
        }
            
        delay(50);
    }

    return (wifiweb && WifiWebMode);
}

void SetWifiWebAppData(String directory) {
    WifiWebAppData = directory;
}

void handleWiFiWeb() {
    if (dnsStarted) {
        dnsServer.processNextRequest();
    }
}
