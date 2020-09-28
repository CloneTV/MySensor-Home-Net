#if !defined(__MY_SENSOR_OPT_H)
#define __MY_SENSOR_OPT_H 1

#define OFF 0U
#define ON  1U
#define STATE 255U

/* Firmware ID, Hostname, Options -> see config.h */

/* BOARD TYPE, posible Atmega328p, ESP-8266 */
#  if defined(ESP8266)
#    include "Wemos_D1_Mini.h"
#    include "ESP2866_mutex.h"
#    define MY_GATEWAY_ESP8266 1
#  elif (defined(__AVR_ATmega328P__) || \
        defined(__AVR_ATmega328__)  || \
        defined(__AVR_ATmega168A__) || \
        defined(__AVR_ATmega168__)  || \
        defined(__AVR_ATmega168P__) || \
        defined(__AVR_ATmega165P__) || \
        defined(__AVR_ATmega169A__) || \
        defined(__AVR_ATmega169__))
#    define __AVR_INTERNAL_LIVE_COMPATIBLE__ 1
#  endif

#include "../config.h"

/* DEBUG */
#  if (!defined(NO_DEBUG) || (defined(NO_DEBUG) && (NO_DEBUG == 0)))
#    define MY_DEBUG
#    if (defined(NO_DEBUG_RADIO) && (NO_DEBUG_RADIO == 0))
#      define MY_SPECIAL_DEBUG
#      define MY_DEBUG_VERBOSE
#      define MY_DEBUG_VERBOSE_RF24
#      define MY_DEBUG_VERBOSE_SIGNING
       /* OTA DEBUG ENABLE */
//#      define MY_DEBUG_OTA 0
#      define MY_DEBUG_VERBOSE_OTA_UPDATE
//#      define MY_OTA_LOG_RECEIVER_FEATURE
//#      define MY_OTA_LOG_SENDER_FEATURE
//#      define MY_DEBUG_OTA_DISABLE_ECHO
#    endif
#  endif
#  if (defined(MY_DEBUG) && !defined(MY_DISABLED_SERIAL))
#    define ENABLE_DEBUG 1
#  endif

/* RADIO NRF24 */
#  define MY_RADIO_RF24
#  if defined(__AVR_INTERNAL_LIVE_COMPATIBLE__)
#    define MY_RF24_CE_PIN 9
#    define MY_RF24_CS_PIN 10
#  elif defined(ESP8266)
#    define MY_RF24_CE_PIN WD4
#    define MY_RF24_CS_PIN WD8
#  endif
#  define MY_RF24_PA_LEVEL RF24_PA_HIGH
#  define MY_RF24_BASE_RADIO_ID 0x00,0xFC,0xE1,0xA8,0xA8
// #  define MY_SIGNAL_REPORT_ENABLED
// #  define MY_RF24_IRQ_PIN 2
// #  define MY_RX_MESSAGE_BUFFER_FEATURE
// #  define MY_RF24_ENABLE_ENCRYPTION
// #  define MY_SMART_SLEEP_WAIT_DURATION_MS (2000UL)
/*
	ESP8266 (wemos mini) -> NRF2401
	D8 - CSN
	D7 - MOSI
	D6 - MISO
	D5 - SCK
	D4 - CE
 */

/* MQTT */
#  if defined(ESP8266)
#    define MY_PORT 1883
#    define MY_NODE_ID 0
#    define MY_GATEWAY_MQTT_CLIENT
// #    define MY_MQTT_CLIENT_PUBLISH_RETAIN
#    define MY_MQTT_PUBLISH_TOPIC_PREFIX "gw-out"
#    define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "gw-in"
#  endif

/* SERIAL ENABLE/DISABLE */
#  if !defined(MY_DEBUG)
#    define MY_DISABLED_SERIAL 1
#    undef MY_BAUD_RATE
#  elif defined(ESP8266)
#    define MY_BAUD_RATE 115200
#  else
#    define MY_BAUD_RATE 57600
#  endif

#  if !defined(DIMMER_SENSOR)
#    define DIMMER_SENSOR 0
#  endif
#  if !defined(LIGHT_SENSOR)
#    define LIGHT_SENSOR 0
#  endif
#  if !defined(LIGHT_SENSOR_BTN)
#    define LIGHT_SENSOR_BTN 0
#  endif

#  if (defined(MY_DEBUG) && !defined(MY_DISABLED_SERIAL))

#    define PRINTINIT() Serial.begin(MY_BAUD_RATE)
#    define PRINTBUILD() {                                                                                                     \
              const PROGMEM char build_sketch[] = "-- Build: %s - %s, %s\n-- Name: [%s:%s]\n-- Init: dimmers=%u, releys=%u, releysBtn=%u\n"; \
              PRINTF(build_sketch,                                                                                             \
                     str_build[0], str_build[1], str_build[2],                                                                 \
                     str_firmware[0], str_firmware[1],                                                                         \
                     DIMMER_SENSOR, LIGHT_SENSOR, LIGHT_SENSOR_BTN); }

#    define PRINT(A) Serial.print(F(A))
#    define PRINTV(A) Serial.print(A)
#    if defined(__AVR_INTERNAL_LIVE_COMPATIBLE__)
#      define PRINTF(A, ...) printf_AVR(A, __VA_ARGS__); Serial.flush()
#    else
#      define PRINTF(A, ...) Serial.printf(A, __VA_ARGS__); Serial.flush()
#    endif
#    define PRINT2(A,B) Serial.print(A,B)
#    define PRINTLN(A) Serial.println(F(A)); Serial.flush()
#  else
#    define PRINTINIT()
#    define PRINTBUILD()
#    define PRINT(A)
#    define PRINTV(A)
#    define PRINTF(A, ...)
#    define PRINT2(A,B)
#    define PRINTLN(A)
#  endif

#  define STR_CONCAT(A,B) STR_CONCAT_(A,B)
#  define STR_CONCAT_(A,B) A ## B
#  define STR_TO_STR(A) STR_TO_STR_(A)
#  define STR_TO_STR_(A) # A
#  define STR_MEM(A) F(A)

#  define __NELE(A) (sizeof(A) / sizeof(A[0]))
#  define SENSOR_POWER_VOLT(A) (A * 0.003363075)
#  define SENSOR_POWER_PCNT(A) (A / 10)

#  if (defined(LED_DEBUG) && (LED_DEBUG == 1))
#    define INIT_LED() pinMode(LED_BUILTIN, OUTPUT)
#    define ERROR_LED(A) __extension__ ({digitalWrite(LED_BUILTIN, HIGH); wait(A); yield(); digitalWrite(LED_BUILTIN, LOW);})
#    define INFO_LED(A) __extension__ ({bool b_ = false; uint8_t cnt_ = A; while (--cnt_ > 0U) { digitalWrite(LED_BUILTIN, (b_) ? HIGH : LOW); b_ = !b_; wait(500); yield();};})
#  else
#    define INIT_LED()
#    define ERROR_LED(A)
#    define INFO_LED(A)
#  endif

/* LOCAL BMP180/BMP280 SENSOR */
#  define BMP280_ADDRESS 0x76
#  define BMP280_ALTITUDE_DEFAULT 1013.25
#  define BMP280_ALTITUDE_INT 101325

///

#  define SENSOR_ID_NONE 255U

///

#  if !defined(INTERNAL_LIVE_IR_SEND_PIN)
#    define INTERNAL_LIVE_IR_SEND_PIN -1
#  endif
#  if !defined(INTERNAL_LIVE_IR_RECEIVE_PIN)
#    define INTERNAL_LIVE_IR_RECEIVE_PIN -1
#  endif
#  if !defined(INTERNAL_LIVE_ILLUMINATION_PIN)
#    define INTERNAL_LIVE_ILLUMINATION_PIN -1
#  endif
#  if !defined(INTERNAL_LIVE_VOLT_PIN)
#    define INTERNAL_LIVE_VOLT_PIN -1
#  endif

#  if !defined(INTERNAL_LIVE_AUTO_FREE_MEM)
#    define INTERNAL_LIVE_AUTO_FREE_MEM 244
#  endif
#  if !defined(INTERNAL_LIVE_AUTO_LIGHT_SETUP)
#    define INTERNAL_LIVE_AUTO_LIGHT_SETUP 245
#  endif
#  if !defined(INTERNAL_LIVE_AUTO_LIGHT)
#    define INTERNAL_LIVE_AUTO_LIGHT 246
#  endif
#  if !defined(INTERNAL_LIVE_IR_SEND)
#    define INTERNAL_LIVE_IR_SEND 247
#  endif
#  if !defined(INTERNAL_LIVE_IR_RECEIVE)
#    define INTERNAL_LIVE_IR_RECEIVE 248
#  endif
#  if !defined(INTERNAL_LIVE_PROXIMITY)
#    define INTERNAL_LIVE_PROXIMITY 249
#  endif
#  if !defined(INTERNAL_LIVE_ILLUMINATION)
#    define INTERNAL_LIVE_ILLUMINATION 250
#  endif
#  if !defined(INTERNAL_LIVE_TEMP)
#    define INTERNAL_LIVE_TEMP 251
#  endif
#  if !defined(INTERNAL_LIVE_BARO)
#    define INTERNAL_LIVE_BARO 252
#  endif
#  if !defined(INTERNAL_LIVE_VOLT)
#    define INTERNAL_LIVE_VOLT 253
#  endif
#  if !defined(INTERNAL_LIVE_RSSI)
#    define INTERNAL_LIVE_RSSI 254
#  endif

#  if defined(MY_DEBUG)
   const PROGMEM char * const str_build[] = {
       __DATE__, __TIME__, __VERSION__
   };
#  endif

const PROGMEM char * const str_firmware[] = {
  MY_HOSTNAME, MY_VERSION
};

#  if (defined(__AVR_INTERNAL_LIVE_COMPATIBLE__) && defined(MY_DEBUG) && !defined(MY_DISABLED_SERIAL))
    static inline void printf_AVR(const char *fmt, ...) {
      char *buf = new char[256]{};
      va_list ap;
      va_start(ap, fmt);
      vsnprintf(buf, 255, fmt, ap);
      for (char *p = &buf[0]; *p; p++) {
        Serial.write(*p);
      }
      va_end(ap);
      delete [] buf;
    }
#  endif

#include <Wire.h>
#include <MySensors.h>
#include "NodeOptionsPWM.h"
#include "NodeInterface.h"
#include "NodeOptionsUtil.h"

/* ------- ENABLE/DISABLE BLOCK ------- */

#   if (INTERNAL_LIVE_ILLUMINATION > 0)
#     if defined(ESP8266)
#       include "NodeI2CLight.h"
#     else
#       include "NodeLiveLight.h"
#     endif
#   endif
#   if (INTERNAL_LIVE_TEMP > 0)
#     if defined(ESP8266)
#       define ENABLE_LIVE_SENSOR_I2C_TEMP 1
#       include "NodeI2CWeather.h"
#     else
#       define MY_AVR_TEMPERATURE_OFFSET 334
#       define MY_AVR_TEMPERATURE_GAIN 0.1
#       define ENABLE_LIVE_SENSOR_TEMP 1
#       include "NodeLiveTemp.h"
#     endif
#   endif
#   if (INTERNAL_LIVE_AUTO_FREE_MEM > 0)
#     define ENABLE_LIVE_FREE_MEM 1
#     include "NodeLiveMem.h"
#   endif
#   if (INTERNAL_LIVE_RSSI > 0)
#     define ENABLE_LIVE_SENSOR_RSSI 1
#     include "NodeLiveRSSI.h"
#   endif
#   if (INTERNAL_LIVE_VOLT_PIN >= 0)
#     define ENABLE_LIVE_SENSOR_VOLT 1
#     include "NodeLiveBat.h"
#   endif
#   if (INTERNAL_LIVE_ILLUMINATION_PIN >= 0)
#     define ENABLE_LIVE_SENSOR_ILLUMINATION 1
#   endif
#   if (LIGHT_SENSOR > 0)
#     define ENABLE_SENSOR_RELAY 1
#     include "NodeActionRelay.h"
#   elif (LIGHT_SENSOR_BTN > 0)
#     define ENABLE_SENSOR_RELAY_BTN 1
#     include "NodeActionRelayButton.h"
#   endif
#   if (DIMMER_SENSOR > 0)
#     define ENABLE_SENSOR_DIMMER 1
#     include "NodeActionDimmer.h"
#   endif
#   if (INTERNAL_LIVE_IR_SEND_PIN > 0)
#     define ENABLE_SENSOR_IR_SEND 1
#   endif
#   if (INTERNAL_LIVE_IR_RECEIVE_PIN >= 0)
#     define ENABLE_SENSOR_IR_RECEIVE 1
#   endif
#   if (defined(ENABLE_SENSOR_IR_SEND) || defined(ENABLE_SENSOR_IR_RECEIVE))
#     define ENABLE_SENSOR_IR 1
#     include "NodeIrControl.h"
#   endif

#endif
