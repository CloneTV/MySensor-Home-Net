// Copyright 2020 Christian Nilsson (@nikize)
// Based on public Arduino BasicOTA example

#if !defined(ESP8266_OTA_H_)
#define ESP8266_OTA_H_

#if defined(ESP8266)
#  include <ESP8266WiFi.h>
#  include <ESP8266mDNS.h>
#  include <WiFiUdp.h>
#  include <ArduinoOTA.h>

#  if defined(MY_DEBUG)
const PROGMEM char * const str_ota[] = {
  "OTA Progress: %u%%\r",
  "Error[%u]: ",
  "OTA error: ",
  "Auth",       // 3
  "Begin",      // 4
  "Connect",    // 5
  "Receive",    // 6
  "End",        // 7
  "\nOTA end",  // 8
  "\nOTA update ",  // 9
  "sketch..",       // 10
  "filesystem..",   // 11
  "IP address: ",   // 12
  "MDNS responder error!", // 13
  "Wifi Connection failed, go to smartConfig mode.", // 14
};
#  endif

void OTAReInit(const char *myhost, led_cb_t errled) {
  if (!MDNS.begin(myhost)) {
    PRINTVLN(str_ota[13]);
    while (1) { ERROR_LEDI2C(errled, 2000); }
  }
  MDNS.addService("http", "tcp", 80);
}

void OTASetup(const char *myhost, const char *mypwd, led_cb_t errled) {
   WiFi.mode(WIFI_STA);
   WiFi.begin(
    static_cast<const char*>(MY_WIFI_SSID),
    static_cast<const char*>(MY_WIFI_PASSWORD)
   );

   INIT_PRINT();
   ArduinoOTA.setHostname(myhost);
   ArduinoOTA.setPassword(mypwd);

   ArduinoOTA.onStart([]() {
    if (ArduinoOTA.getCommand() == U_FLASH) {
      PRINTV(str_ota[9]); PRINTVLN(str_ota[10]);
    } else {
      PRINTV(str_ota[9]); PRINTVLN(str_ota[11]);
    }
   });
   ArduinoOTA.onEnd([]() {
    PRINTVLN(str_ota[8]);
   });
   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    PRINTF(str_ota[0], (progress / (total / 100)));
   });
   ArduinoOTA.onError([](ota_error_t error) {
    PRINTF(str_ota[1], error);
    if (error == OTA_AUTH_ERROR) {
      PRINTV(str_ota[2]); PRINTVLN(str_ota[3]);
    } else if (error == OTA_BEGIN_ERROR) {
      PRINTV(str_ota[2]); PRINTVLN(str_ota[4]);
    } else if (error == OTA_CONNECT_ERROR) {
      PRINTV(str_ota[2]); PRINTVLN(str_ota[5]);
    } else if (error == OTA_RECEIVE_ERROR) {
      PRINTV(str_ota[2]); PRINTVLN(str_ota[6]);
    } else if (error == OTA_END_ERROR) {
      PRINTV(str_ota[2]); PRINTVLN(str_ota[7]);
    }
   });
   ArduinoOTA.begin();

  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    PRINTV(str_ota[12]);
    PRINTVLN(WiFi.localIP());
    PRINT("\n");
  } else {
    PRINTVLN(str_ota[14]);
    ERROR_LEDI2C(errled, 1000);
    WiFi.disconnect();
    WiFi.beginSmartConfig();
    uint8_t cnt = 0U;
    while ((WiFi.status() != WL_CONNECTED) && (++cnt < 200)) {
      ERROR_LEDI2C(errled, 1000);
      PRINTV(".");
    }
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();
      ESP.restart();
      while(true) { yield(); };
    }
    PRINTV(WiFi.smartConfigDone());
  }
  if (!WiFi.getAutoConnect())
    WiFi.setAutoConnect(true);
  if (!WiFi.getAutoReconnect())
    WiFi.setAutoReconnect(true);

  OTAReInit(myhost, errled);
}

void OTAHandler() {
  ArduinoOTA.handle();
}

#  endif
#endif
