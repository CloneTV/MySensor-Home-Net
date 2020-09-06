
#if !defined(__MY_GATEWAY_ESP_H)
#define __MY_GATEWAY_ESP_H 1

#include "wemos_D1_mini.h"
#include "config.h"

/* WI-FI -> config.h */
/* MQTT -> config.h */
/* Firmware ID, Hostname -> config.h */

/* DEBUG */
#  if !defined(NO_DEBUG)
#    define MY_DEBUG
#    define MY_SPECIAL_DEBUG
#    define MY_DEBUG_VERBOSE
#    define MY_DEBUG_VERBOSE_RF24
#    define MY_DEBUG_VERBOSE_SIGNING
#  endif

/* RADIO NRF24 */
#  define MY_RADIO_RF24
#  define MY_RF24_CE_PIN WD4
#  define MY_RF24_CS_PIN WD8
#  define MY_RF24_PA_LEVEL RF24_PA_HIGH
// #  define MY_RF24_ENABLE_ENCRYPTION

#  define MY_GATEWAY_ESP8266
#  define MY_GATEWAY_MAX_CLIENTS 8
#  define MY_REPEATER_FEATURE
//#  define MY_SMART_SLEEP_WAIT_DURATION_MS (2000UL)

/* INCLUSION BUTTON */
#define MY_INCLUSION_MODE_FEATURE
#define MY_INCLUSION_BUTTON_FEATURE
#define MY_INCLUSION_MODE_BUTTON_PIN WRX
#define MY_INCLUSION_MODE_DURATION 60
//

/* SERIAL ENABLE/DISABLE */
#  if !defined(MY_DEBUG)
#    define MY_DEFAULT_ERR_LED_PIN WTX
#    define MY_DISABLED_SERIAL 1
#    undef MY_BAUD_RATE
#  else
#    define MY_BAUD_RATE 115200
#  endif

/* MQTT */
#  define MY_PORT 1883
#  define MY_NODE_ID 0
#  define MY_GATEWAY_MQTT_CLIENT
#  define MY_MQTT_PUBLISH_TOPIC_PREFIX "gw-out"
#  define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "gw-in"

/* MYSENSOR OTA ENABLE */
#  define MY_OTA_LOG_RECEIVER_FEATURE
#  define MY_OTA_FIRMWARE_FEATURE

/* LOCAL SENSOR */
#  define NODE_SENSOR_IR_SEND 0
#  define NODE_SENSOR_IR_RECV 1
#  define NODE_SENSOR_WETHER_TEMP 2
#  define NODE_SENSOR_WETHER_BARO 3

/* LOCAL IR SEND/RECIVE */
#  define PIN_SENSOR_IR_SEND WD0 // WTX ? change
#  define PIN_SENSOR_IR_RECV WD3

/* LOCAL BMP180 SENSOR */
#  define BMP_ADDRESS 0x76
#  define BMP_ALTITUDE_DEFAULT 1013.25
#  define BMP_ALTITUDE_INT 101325

#  if (defined(MY_DEBUG) && !defined(MY_DISABLED_SERIAL))
#    define PRINT(A) Serial.print(F(A))
#    define PRINTV(A) Serial.print(A)
#    define PRINTF(A, ...) Serial.printf(A, __VA_ARGS__); Serial.flush()
#    define PRINT2(A,B) Serial.print(A,B)
#    define PRINTLN(A) Serial.println(F(A)); Serial.flush()
#  else
#    define PRINT(A)
#    define PRINTV(A)
#    define PRINTF(A, ...)
#    define PRINT2(A,B)
#    define PRINTLN(A)
#  endif
#  define STR_MEM(A) F(A)

#endif
