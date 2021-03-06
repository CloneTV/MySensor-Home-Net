#if !defined(__MY_SENSOR_OPT_H)
#define __MY_SENSOR_OPT_H 1

#define OFF 0U
#define ON  1U
#define STATE 255U

/* Firmware ID, Hostname, Options -> see config.h */

/* BOARD TYPE, posible Atmega328p, ESP-8266 */
#  if defined(ESP8266)
#    include "Wemos_D1_Mini.h"
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

#  include "I2CDefine.h"
#  include "../config.h"

#  if defined(__AVR_INTERNAL_LIVE_COMPATIBLE__)
#    define TOMEM_ 
#  else
#    define TOMEM_ PROGMEM
#  endif

/* SOIL SENSOR, BAT POWER */
#   if defined(SOIL_SENSOR_PINS)
#     define MY_PASSIVE_NODE
#     if defined(__AVR_INTERNAL_LIVE_COMPATIBLE__)
#       define MY_PASSIVE_BTN_PIN 3
#     else
#       define MY_PASSIVE_BTN_PIN 2
#     endif
#     if !defined(MY_TIME_SLEEPS)
#       define MY_TIME_SLEEPS 3600000 // 1 hour
#     endif
#     if defined(NO_DEBUG)
#       undef NO_DEBUG
#     endif
#     define NO_DEBUG 1
#   endif


/* INCLUSION BUTTON, LEDS */
#  if defined(I2C_PCF8574_ENABLE)
#    define MY_DEFAULT_LED_I2C 1
#    if defined(MY_INCLUSION_BUTTON_FEATURE)
#      define MY_INCLUSION_MODE_FEATURE
#      define MY_INCLUSION_MODE_DURATION 10
#      define MY_INCLUSION_BUTTON_TIMEOUT 500
#    endif
#  endif

/* DEBUG - SERIAL ENABLE/DISABLE */
#  if (!defined(NO_DEBUG) || (defined(NO_DEBUG) && (NO_DEBUG == 0)))
#    define MY_DEBUG
#    if defined(ESP8266)
#      define MY_BAUD_RATE 115200
#    else
#      define MY_BAUD_RATE 57600
#    endif
#    if (defined(NO_DEBUG_RADIO) && (NO_DEBUG_RADIO == 0))
#      define MY_SPECIAL_DEBUG
#      define MY_DEBUG_VERBOSE
#      define MY_DEBUG_VERBOSE_RF24
#      define MY_DEBUG_VERBOSE_SIGNING
       /* OTA DEBUG ENABLE */
#      if !defined(ESP8266)
#        define MY_DEBUG_VERBOSE_OTA_UPDATE
#      endif
//#      define MY_DEBUG_OTA 0
//#      define MY_OTA_LOG_RECEIVER_FEATURE
//#      define MY_OTA_LOG_SENDER_FEATURE
//#      define MY_DEBUG_OTA_DISABLE_ECHO
#    endif
#  else
#    define MY_DISABLED_SERIAL 1
#    undef MY_BAUD_RATE
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
	0 => 2400 Mhz (RF24 channel 1)
	1 => 2401 Mhz (RF24 channel 2)
	76 => 2476 Mhz (RF24 channel 77) standard
	83 => 2483 Mhz (RF24 channel 84)
	124 => 2524 Mhz (RF24 channel 125)
	125 => 2525 Mhz (RF24 channel 126)
*/
/*
	ESP8266 (wemos mini) -> NRF2401
	D8 - CSN  -> (v+)2
	D7 - MOSI -> (v+)3
	D6 - MISO -> (v-)4
	D5 - SCK  -> (v-)3
	D4 - CE   -> (v-)2
        V+  -> 1
        V-  -> 1

	ESP8266 (wemos d1-r1) -> NRF2401
	D10 - CSN  -> (v+)2
	D13 - MOSI -> (v+)3
	D12 - MISO -> (v-)4
	D14 - SCK  -> (v-)3
	D9  - CE   -> (v-)2
        V+  -> 1
        V-  -> 1

  Pro Mini (328p) -> NRF2401
  9  - CE   -> (v-)2
  10 - CSN  -> (v+)2
  13 - SCK  -> (v-)3
  11 - MOSI -> (v+)3
  12 - MISO -> (v-)4
        V+  -> 1
        V-  -> 1

  Nano, Mega -> NRF2401
  49 - CE   -> (v-)2
  53 - CSN  -> (v+)2
  52 - SCK  -> (v-)3
  51 - MOSI -> (v+)3
  50 - MISO -> (v-)4
        V+  -> 1
        V-  -> 1

 */

/* MQTT */
#  if defined(ESP8266)
#    define MY_PORT 1883
#    define MY_NODE_ID 0
#    define MY_GATEWAY_MQTT_CLIENT
#    define MY_MQTT_PUBLISH_TOPIC_PREFIX "gw-out"
#    define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "gw-in"
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

#    if defined(ESP8266)
#      define INIT_PRINT() { Serial.begin(MY_BAUD_RATE, SERIAL_8N1, SERIAL_TX_ONLY); while (!Serial) delay(50); }
#    else
#      define INIT_PRINT() Serial.begin(MY_BAUD_RATE)
#    endif
#    define BUILD_PRINT() {                                                                                                     \
              const TOMEM_ char build_sketch[] = "-- Build: %s - %s, %s\n-- Name: [%s:%s]\n-- Init: dimmers=%u, releys=%u, releysBtn=%u\n"; \
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
#    define PRINTVLN(A) Serial.println(A)
#  else
#    define INIT_PRINT()
#    define BUILD_PRINT()
#    define PRINT(A)
#    define PRINTV(A)
#    define PRINTF(A, ...)
#    define PRINT2(A,B)
#    define PRINTLN(A)
#    define PRINTVLN(A)
#  endif

#  define STR_CONCAT(A,B) STR_CONCAT_(A,B)
#  define STR_CONCAT_(A,B) A ## B
#  define STR_TO_STR(A) STR_TO_STR_(A)
#  define STR_TO_STR_(A) # A
#  define STR_MEM(A) F(A)

#  define __NELE(A) (sizeof(A) / sizeof(A[0]))
#  define SENSOR_POWER_VOLT(A) (A * 0.003363075)
#  define SENSOR_POWER_PCNT(A) (A / 10)

typedef void (*led_cb_t)(uint8_t);

#  if (defined(LED_DEBUG) && (LED_DEBUG == 1))
#    define INIT_LED() pinMode(LED_BUILTIN, OUTPUT)
#    define ERROR_LED(A) __extension__ ({digitalWrite(LED_BUILTIN, HIGH); wait(A); yield(); digitalWrite(LED_BUILTIN, LOW);})
#    define ERROR_LEDI2C(A,B) ERROR_LED(B)
#    define INFO_LED(A) __extension__ ({bool b_ = false; uint8_t cnt_ = A; while (--cnt_ > 0U) { digitalWrite(LED_BUILTIN, (b_) ? HIGH : LOW); b_ = !b_; wait(500); yield();};})

#  elif (defined(ESP8266) && defined(I2C_PCF8574_ENABLE))
#    define INIT_LED()
#    define ERROR_LED(A) ERROR_LEDI2C(nled.errorLed, A)
#    define ERROR_LEDI2C(A,B) __extension__ ({ A(LOW); delay(B); A(HIGH); delay(B / 2);})
#    define INFO_LED(A) __extension__ ({uint8_t cnt_ = A, b_ = HIGH; while (--cnt_ > 0U) { nled.infoLed(b_); b_ = ((b_ == LOW) ? HIGH : LOW); delay(750);} b_ = LOW; nled.infoLed(b_);})

#  else
#    define INIT_LED()
#    define ERROR_LED(A)
#    define INFO_LED(A)
#  endif

/* LOCAL BMP180/BMP280 SENSOR */
#  if defined(BMP280_ADDRESS)
#    undef BMP280_ADDRESS
#  endif
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

#  if !defined(INTERNAL_LIVE_VIRTUAL)
#    define INTERNAL_LIVE_VIRTUAL 199
#  endif
#  if !defined(INTERNAL_LIVE_INFO)
#    define INTERNAL_LIVE_INFO 241
#  endif
#  if !defined(INTERNAL_LIVE_SOIL)
#    define INTERNAL_LIVE_SOIL 242
#  endif
#  if !defined(INTERNAL_LIVE_CMD_REBOOT)
#    define INTERNAL_LIVE_CMD_REBOOT 243
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
   const TOMEM_ char * const str_build[] = {
       __DATE__, __TIME__, __VERSION__
   };
#  endif

const TOMEM_ char * const str_firmware[] = {
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

#  include <Wire.h>
#  if defined(ESP8266)
#    include "ESP2866_mutex.h"
#    include "ESP8266_OTA.h"
#  endif
#  include <MySensors.h>
#  include <core/MyInclusionMode.h>
#  include "NodeOptionsPWM.h"
#  include "NodeInterface.h"
#  include "NodeOptionsUtil.h"

/* ------- ENABLE/DISABLE BLOCK ------- */

#   if (defined(INTERNAL_LIVE_ILLUMINATION_PIN) && (INTERNAL_LIVE_ILLUMINATION_PIN >= 0))
#     if (defined(I2C_AP3216_ENABLE) && defined(ESP8266))
#       define ENABLE_I2C_SENSOR_ILLUMINATION 1
#       include "NodeI2CLight.h"
#     else
#       define ENABLE_LIVE_SENSOR_ILLUMINATION 1
#       include "NodeLiveLight.h"
#     endif
#   endif
#   if (defined(INTERNAL_LIVE_TEMP) && (INTERNAL_LIVE_TEMP > 0))
#     if (defined(I2C_BMP180_ENABLE) && defined(ESP8266))
#       define ENABLE_I2C_SENSOR_TEMP 1
#       include "NodeI2CWeather.h"
#     else
#       if !defined(MY_AVR_TEMPERATURE_OFFSET)
#         define MY_AVR_TEMPERATURE_OFFSET 334
#       endif
#       if !defined(MY_AVR_TEMPERATURE_GAIN)
#         define MY_AVR_TEMPERATURE_GAIN 0.1
#       endif
#       define ENABLE_LIVE_SENSOR_TEMP 1
#       include "NodeLiveTemp.h"
#     endif
#   endif
#   if defined(I2C_PCF8574_ENABLE)
#     define ENABLE_I2C_GPIO_EXPANDER 1
#     include "NodeI2CExpander.h"
#   endif
#   if defined(ENABLE_LIVE_FREE_MEM)
#     include "NodeLiveMem.h"
#   endif
#   if defined(ENABLE_LIVE_SENSOR_RSSI)
#     include "NodeLiveRSSI.h"
#   endif
#   if (defined(INTERNAL_LIVE_VOLT_PIN) && (INTERNAL_LIVE_VOLT_PIN >= 0))
#     define ENABLE_LIVE_SENSOR_VOLT 1
#     include "NodeLiveBat.h"
#   endif
#   if (defined(LIGHT_SENSOR) && (LIGHT_SENSOR > 0))
#     define ENABLE_SENSOR_RELAY 1
#     include "NodeActionRelay.h"
#   elif (defined(LIGHT_SENSOR_BTN) && (LIGHT_SENSOR_BTN > 0))
#     define ENABLE_SENSOR_RELAY_BTN 1
#     include "NodeActionRelayButton.h"
#   endif
#   if defined(SOIL_SENSOR_PINS)
#     define ENABLE_SENSOR_SOIL 1
#     include "NodeActionMoisture.h"
#   endif
#   if (defined(DIMMER_SENSOR) && (DIMMER_SENSOR > 0))
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
#   endif
#   if (defined(INTERNAL_RF433_PIN) && (INTERNAL_RF433_PIN >= 0))
#     define ENABLE_SENSOR_RF433 1
#     include "NodeCommand.h"
#   endif
#   if (defined(ENABLE_SENSOR_IR) && !defined(__MY_SENSOR_NODE_CMD_H))
#       include "NodeIrControl.h"
#   endif

#endif
