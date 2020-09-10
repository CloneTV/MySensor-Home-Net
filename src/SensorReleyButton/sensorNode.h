#if !defined(__MY_SENSOR_NODE_H)
#define __MY_SENSOR_NODE_H 1

#include "config.h"

/* Firmware ID, Hostname -> config.h */

/* DEBUG */
#  if (!defined(NO_DEBUG) || (defined(NO_DEBUG) && (NO_DEBUG == 0)))
#    define MY_DEBUG
#    if (defined(NO_DEBUG_RADIO) && (NO_DEBUG_RADIO == 0))
#      define MY_SPECIAL_DEBUG
#      define MY_DEBUG_VERBOSE
#      define MY_DEBUG_VERBOSE_RF24
#      define MY_DEBUG_VERBOSE_SIGNING
       /* OTA DEBUG ENABLE */
#      define MY_DEBUG_OTA 0
#      define MY_DEBUG_VERBOSE_OTA_UPDATE
#      define MY_OTA_LOG_RECEIVER_FEATURE
#      define MY_OTA_LOG_SENDER_FEATURE
#      define MY_DEBUG_OTA_DISABLE_ECHO
#    endif
#  endif

/* RADIO NRF24 */
#  define MY_RADIO_RF24
#  define MY_RF24_CE_PIN 9
#  define MY_RF24_CS_PIN 10
#  define MY_RF24_PA_LEVEL RF24_PA_HIGH
#  define MY_RF24_BASE_RADIO_ID 0x00,0xFC,0xE1,0xA8,0xA8
// #  define MY_RF24_ENABLE_ENCRYPTION

#  define MY_REPEATER_FEATURE
#  define MY_SMART_SLEEP_WAIT_DURATION_MS (2000UL)

/* SERIAL ENABLE/DISABLE */
#  if !defined(MY_DEBUG)
#    define MY_DISABLED_SERIAL 1
#    undef MY_BAUD_RATE
#  else
#    define MY_BAUD_RATE 57600
#  endif

#  if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__))
#    define AVR328 1
#  endif

#  if (defined(MY_DEBUG) && !defined(MY_DISABLED_SERIAL))
#    define PRINTINIT() Serial.begin(MY_BAUD_RATE)
#    define PRINT(A) Serial.print(F(A))
#    define PRINTV(A) Serial.print(A)
#    if defined(AVR328)
#      define PRINTF(A, ...) printf_avr328(A, __VA_ARGS__); Serial.flush()
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

#  define STR_MEM(A) F(A)
#  define SENSOR_SET(A) ((A > 0U) ? LOW : HIGH)
#  define SENSOR_INIT(A) ({ uint8_t a = static_cast<uint8_t>(A); ((a > 1U) ? 0U : a);})

#  if (defined(LED_DEBUG) && (LED_DEBUG == 1))
#    define ERROR_LED(A) __extension__ ({digitalWrite(LED_BUILTIN, HIGH); delay(A); yield(); digitalWrite(LED_BUILTIN, LOW);})
#    define INFO_LED(A) __extension__ ({bool b_ = false; uint8_t cnt_ = A; while (--cnt_ > 0U) { digitalWrite(LED_BUILTIN, (b_) ? HIGH : LOW); b_ = !b_; delay(500); yield();};})
#  else
#    define ERROR_LED(A)
#    define INFO_LED(A)
#  endif

///

#  define INDEX_NODE_SENSOR_ID 0
#  define INDEX_PIN_SENSOR_BUTTON 1
#  define INDEX_PIN_SENSOR_RELAY 2

#  if !defined(PIN_INTERNAL_LIVE_VOLT)
#    define PIN_INTERNAL_LIVE_VOLT 0
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

#  if !defined(AVR328)
#    undef INTERNAL_LIVE_TEMP
#    define INTERNAL_LIVE_TEMP 0
#  endif

///

#  if defined(LIGHT_SENSOR1)

#    define NODE_SENSOR_ID_1 0
#    define PIN_SENSOR_BUTTON_1 4
#    define PIN_SENSOR_RELAY_1 5
#    define size_sensors 1U

     const PROGMEM uint16_t setup_sensors[][3] = {
        { NODE_SENSOR_ID_1, PIN_SENSOR_BUTTON_1, PIN_SENSOR_RELAY_1 }
     };

#  elif defined(LIGHT_SENSOR2)

#    define NODE_SENSOR_ID_1 0
#    define NODE_SENSOR_ID_2 1
#    define PIN_SENSOR_BUTTON_1 4
#    define PIN_SENSOR_BUTTON_2 5
#    define PIN_SENSOR_RELAY_1 6
#    define PIN_SENSOR_RELAY_2 7
#    define size_sensors 2U

     const PROGMEM uint16_t setup_sensors[][3] = {
        { NODE_SENSOR_ID_1, PIN_SENSOR_BUTTON_1, PIN_SENSOR_RELAY_1 },
        { NODE_SENSOR_ID_2, PIN_SENSOR_BUTTON_2, PIN_SENSOR_RELAY_2 }
     };

#  elif defined(LIGHT_SENSOR3)

#    define NODE_SENSOR_ID_1 0
#    define NODE_SENSOR_ID_2 1
#    define NODE_SENSOR_ID_3 2
#    define PIN_SENSOR_BUTTON_1 4
#    define PIN_SENSOR_BUTTON_2 5
#    define PIN_SENSOR_BUTTON_3 6
#    define PIN_SENSOR_RELAY_1 7
#    define PIN_SENSOR_RELAY_2 8
#    define PIN_SENSOR_RELAY_3 3
#    define size_sensors 3U

     const PROGMEM uint16_t setup_sensors[][3] = {
        { NODE_SENSOR_ID_1, PIN_SENSOR_BUTTON_1, PIN_SENSOR_RELAY_1 },
        { NODE_SENSOR_ID_2, PIN_SENSOR_BUTTON_2, PIN_SENSOR_RELAY_2 },
        { NODE_SENSOR_ID_3, PIN_SENSOR_BUTTON_3, PIN_SENSOR_RELAY_3 }
     };

#  endif

const PROGMEM char * const str_firmware[] = {
  MY_HOSTNAME, MY_VERSION
};

#  if (defined(AVR328) && defined(MY_DEBUG) && !defined(MY_DISABLED_SERIAL))
    static inline void printf_avr328(const char *fmt, ...) {
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

#endif
