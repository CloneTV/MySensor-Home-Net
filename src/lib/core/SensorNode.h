#if !defined(__MY_SENSOR_NODE_H)
#define __MY_SENSOR_NODE_H 1

#define OFF 0U
#define ON  1U
#define ALL 255U

#include "../config.h"

/* Firmware ID, Hostname, Options -> see config.h */

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
#  define MY_RF24_CE_PIN 9
#  define MY_RF24_CS_PIN 10
#  define MY_RF24_PA_LEVEL RF24_PA_HIGH
#  define MY_RF24_BASE_RADIO_ID 0x00,0xFC,0xE1,0xA8,0xA8
#  define MY_SIGNAL_REPORT_ENABLED
// #  define MY_RF24_IRQ_PIN 2
// #  define MY_RX_MESSAGE_BUFFER_FEATURE
// #  define MY_RF24_ENABLE_ENCRYPTION
// #  define MY_SMART_SLEEP_WAIT_DURATION_MS (2000UL)

/* SERIAL ENABLE/DISABLE */
#  if !defined(MY_DEBUG)
#    define MY_DISABLED_SERIAL 1
#    undef MY_BAUD_RATE
#  else
#    define MY_BAUD_RATE 57600
#  endif

#  if (defined(__AVR_ATmega328P__) || \
       defined(__AVR_ATmega328__)  || \
       defined(__AVR_ATmega168A__) || \
       defined(__AVR_ATmega168__)  || \
       defined(__AVR_ATmega168P__) || \
       defined(__AVR_ATmega165P__) || \
       defined(__AVR_ATmega169A__) || \
       defined(__AVR_ATmega169__))
#    define __AVR_INTERNAL_LIVE_COMPATIBLE__ 1
#  endif

#  if (defined(MY_DEBUG) && !defined(MY_DISABLED_SERIAL))
#    define PRINTINIT() Serial.begin(MY_BAUD_RATE)
#    define PRINT(A) Serial.print(F(A))
#    define PRINTV(A) Serial.print(A)
#    if defined(__AVR_INTERNAL_LIVE_COMPATIBLE__)
#      define PRINTF(A, ...) printf___AVR_INTERNAL_LIVE_COMPATIBLE__(A, __VA_ARGS__); Serial.flush()
#    else
#      define PRINTF(A, ...) Serial.printf(A, __VA_ARGS__); Serial.flush()
#    endif
#    define PRINT2(A,B) Serial.print(A,B)
#    define PRINTLN(A) Serial.println(F(A)); Serial.flush()
#  else
#    define PRINTINIT()
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
#  define SENSOR_SET(A) ((A > 0U) ? LOW : HIGH)
#  define SENSOR_INIT(A) ({ uint8_t a = static_cast<uint8_t>(A); ((a > 1U) ? 0U : a);})
#  define SENSOR_POWER_VOLT(A) (A * 0.003363075)
#  define SENSOR_POWER_PCNT(A) (A / 10)

#  if (defined(LED_DEBUG) && (LED_DEBUG == 1))
#    define INIT_LED() pinMode(LED_BUILTIN, OUTPUT)
#    define ERROR_LED(A) __extension__ ({digitalWrite(LED_BUILTIN, HIGH); delay(A); yield(); digitalWrite(LED_BUILTIN, LOW);})
#    define INFO_LED(A) __extension__ ({bool b_ = false; uint8_t cnt_ = A; while (--cnt_ > 0U) { digitalWrite(LED_BUILTIN, (b_) ? HIGH : LOW); b_ = !b_; delay(500); yield();};})
#  else
#    define INIT_LED()
#    define ERROR_LED(A)
#    define INFO_LED(A)
#  endif

///

#  define INDEX_NODE_SENSOR_ID    0
#  define INDEX_PIN_SENSOR_BUTTON 1
#  define INDEX_PIN_SENSOR_DIMMER 1
#  define INDEX_PIN_SENSOR_RELAY  2

#  if !defined(INTERNAL_LIVE_ILLUMINATION_PIN)
#    define INTERNAL_LIVE_ILLUMINATION_PIN -1
#  endif
#  if !defined(INTERNAL_LIVE_VOLT_PIN)
#    define INTERNAL_LIVE_VOLT_PIN -1
#  endif
#  if !defined(INTERNAL_LIVE_ILLUMINATION)
#    define INTERNAL_LIVE_ILLUMINATION 251
#  endif
#  if !defined(INTERNAL_LIVE_VOLT)
#    define INTERNAL_LIVE_VOLT 252
#  endif
#  if !defined(INTERNAL_LIVE_TEMP)
#    define INTERNAL_LIVE_TEMP 253
#  endif
#  if !defined(INTERNAL_LIVE_RSSI)
#    define INTERNAL_LIVE_RSSI 254
#  endif

#  if !defined(__AVR_INTERNAL_LIVE_COMPATIBLE__)
#    undef INTERNAL_LIVE_TEMP
#    define INTERNAL_LIVE_TEMP 0
#  endif

/* ------- LIGHT_SENSOR ------- */

#  if defined(LIGHT_SENSOR)
#    if (LIGHT_SENSOR == 1)

#      define NODE_SENSOR_LIGHT_ID_1 0
#      define PIN_SENSOR_BUTTON_1    4
#      define PIN_SENSOR_RELAY_1     7
#      define size_light_sensors     1U

       const PROGMEM uint16_t setup_light_sensors[][3] = {
          { NODE_SENSOR_LIGHT_ID_1, PIN_SENSOR_BUTTON_1, PIN_SENSOR_RELAY_1 }
       };

#    elif (LIGHT_SENSOR == 2)

#      define NODE_SENSOR_LIGHT_ID_1 0
#      define NODE_SENSOR_LIGHT_ID_2 1
#      define PIN_SENSOR_BUTTON_1    4
#      define PIN_SENSOR_BUTTON_2    2
#      define PIN_SENSOR_RELAY_1     7
#      define PIN_SENSOR_RELAY_2     8
#      define size_light_sensors     2U

       const PROGMEM uint16_t setup_light_sensors[][3] = {
          { NODE_SENSOR_LIGHT_ID_1, PIN_SENSOR_BUTTON_1, PIN_SENSOR_RELAY_1 },
          { NODE_SENSOR_LIGHT_ID_2, PIN_SENSOR_BUTTON_2, PIN_SENSOR_RELAY_2 }
       };

#    elif (LIGHT_SENSOR == 3)

#      define NODE_SENSOR_LIGHT_ID_1 0
#      define NODE_SENSOR_LIGHT_ID_2 1
#      define NODE_SENSOR_LIGHT_ID_3 2
#      define PIN_SENSOR_BUTTON_1    4
#      define PIN_SENSOR_BUTTON_2    5
#      define PIN_SENSOR_BUTTON_3    6
#      define PIN_SENSOR_RELAY_1     7
#      define PIN_SENSOR_RELAY_2     8
#      define PIN_SENSOR_RELAY_3     3
#      define size_light_sensors     3U

       const PROGMEM uint16_t setup_light_sensors[][3] = {
          { NODE_SENSOR_LIGHT_ID_1, PIN_SENSOR_BUTTON_1, PIN_SENSOR_RELAY_1 },
          { NODE_SENSOR_LIGHT_ID_2, PIN_SENSOR_BUTTON_2, PIN_SENSOR_RELAY_2 },
          { NODE_SENSOR_LIGHT_ID_3, PIN_SENSOR_BUTTON_3, PIN_SENSOR_RELAY_3 }
       };

#      if defined(DIMMER_SENSOR)
#        pragma message "DIMMER SENSOR + LIGHT SENSOR == 3 NOT FREE PINS!"
#      endif

#    endif
#  else
#    define size_light_sensors 0U
#  endif

/* ------- DIMMER_SENSOR ------- */

#  if defined(DIMMER_SENSOR)
#    if (DIMMER_SENSOR == 1)

#      define NODE_SENSOR_DIMMER_ID_1 (size_light_sensors + 1U)
#      define PIN_SENSOR_DIMMER_1 5
#      define size_dimmer_sensors 1U

       const PROGMEM uint16_t setup_dimmer_sensors[][2] = {
          { NODE_SENSOR_DIMMER_ID_1, PIN_SENSOR_DIMMER_1 }
       };

#    elif (DIMMER_SENSOR == 2)

#      define NODE_SENSOR_DIMMER_ID_1 (size_light_sensors + 1U)
#      define NODE_SENSOR_DIMMER_ID_2 (size_light_sensors + 2U)
#      define PIN_SENSOR_DIMMER_1 5
#      define PIN_SENSOR_DIMMER_2 6
#      define size_dimmer_sensors 2U

       const PROGMEM uint16_t setup_dimmer_sensors[][2] = {
          { NODE_SENSOR_DIMMER_ID_1, PIN_SENSOR_DIMMER_1 },
          { NODE_SENSOR_DIMMER_ID_2, PIN_SENSOR_DIMMER_2 }
       };

#    elif (DIMMER_SENSOR == 3)

#      define NODE_SENSOR_DIMMER_ID_1 (size_light_sensors + 1U)
#      define NODE_SENSOR_DIMMER_ID_2 (size_light_sensors + 2U)
#      define NODE_SENSOR_DIMMER_ID_3 (size_light_sensors + 3U)
#      define PIN_SENSOR_DIMMER_1 5
#      define PIN_SENSOR_DIMMER_2 6
#      define PIN_SENSOR_DIMMER_3 3
#      define size_dimmer_sensors 3U

       const PROGMEM uint16_t setup_dimmer_sensors[][2] = {
          { NODE_SENSOR_DIMMER_ID_1, PIN_SENSOR_DIMMER_1 },
          { NODE_SENSOR_DIMMER_ID_2, PIN_SENSOR_DIMMER_2 },
          { NODE_SENSOR_DIMMER_ID_3, PIN_SENSOR_DIMMER_3 }
       };

#    endif
#  else
#    define size_dimmer_sensors 0U
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
    static inline void printf___AVR_INTERNAL_LIVE_COMPATIBLE__(const char *fmt, ...) {
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
#include "presenter.h"

/* ------- INFO DEBUG BLOCK ------- */

#   if ((INTERNAL_LIVE_TEMP > 0) && !defined(MY_GATEWAY_ESP8266))
#     define MY_AVR_TEMPERATURE_OFFSET 334
#     define MY_AVR_TEMPERATURE_GAIN 0.1
#     define ENABLE_LIVE_SENSOR_TEMP 1
#   endif
#   if (INTERNAL_LIVE_RSSI > 0)
#     define ENABLE_LIVE_SENSOR_RSSI 1
#   endif
#   if (defined(INTERNAL_LIVE_VOLT_PIN) && (INTERNAL_LIVE_VOLT_PIN >= 0))
#     define ENABLE_LIVE_SENSOR_VOLT 1
#   endif
#   if (defined(INTERNAL_LIVE_ILLUMINATION_PIN) && (INTERNAL_LIVE_ILLUMINATION_PIN >= 0))
#     define ENABLE_LIVE_SENSOR_ILLUMINATION 1
#   endif

#   if defined(MY_DEBUG)
#     if defined(ENABLE_LIVE_SENSOR_ILLUMINATION)
#       pragma message "INTERNAL (ILLUMINATION [fotoresistor]) COMPILED! SENSOR ID=251"
#     endif
#     if defined(ENABLE_LIVE_SENSOR_VOLT)
#       pragma message "INTERNAL (VOLT) COMPILED! SENSOR ID=252"
#     endif
#     if defined(ENABLE_LIVE_SENSOR_TEMP)
#       pragma message "INTERNAL (TEMP) COMPILED! SENSOR ID=253"
#     endif
#     if defined(ENABLE_LIVE_SENSOR_RSSI)
#       pragma message "INTERNAL (RSSI) COMPILED! SENSOR ID=254"
#     endif

#     if defined(LIGHT_SENSOR)
#       if (LIGHT_SENSOR == 1)
#          pragma message "LIGHT SENSORS NUMBER=1, BTN PIN=4, RELEY PIN=7"
#       elif (LIGHT_SENSOR == 2)
#          pragma message "LIGHT SENSORS NUMBER=2, BTN PIN=4,2, RELEY PIN=7,8"
#       elif (LIGHT_SENSOR == 3)
#          pragma message "LIGHT SENSORS NUMBER=3, BTN PIN=4,5,6 RELEY PIN=7,8,3"
#       endif
#     endif
#     if defined(DIMMER_SENSOR)
#       if (DIMMER_SENSOR == 1)
#          pragma message "DIMMER SENSORS NUMBER=1, DIMMER PIN=5"
#       elif (DIMMER_SENSOR == 2)
#          pragma message "DIMMER SENSORS NUMBER=2, DIMMER PIN=5,6"
#       elif (DIMMER_SENSOR == 3)
#          pragma message "DIMMER SENSORS NUMBER=3, DIMMER PIN=5,6,3"
#       endif
#     endif
#   endif

#endif
