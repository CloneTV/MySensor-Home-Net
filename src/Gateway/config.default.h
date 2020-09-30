
#if !defined(__MY_GW_CONFIG_ESP_H)
#define __MY_GW_CONFIG_ESP_H 1

#  if 0


#if !defined(__MY_GW_CONFIG_ESP_H)
#define __MY_GW_CONFIG_ESP_H 1

/* WI-FI */
#  define MY_WIFI_SSID "ssid"
#  define MY_WIFI_PASSWORD "pwd"
#  define MY_OTA_PASSWD "otapwd"
/* Firmware ID */
#  define MY_HOSTNAME "esp8266-Gateway"
#  define MY_VERSION "1.3"
/* MQTT */
#  define MY_CONTROLLER_IP_ADDRESS 192, 168, 0, 2
#  define MY_MQTT_CLIENT_ID "group id"
#  define MY_MQTT_USER "user id"
#  define MY_MQTT_PASSWORD "user pwd"
// #  define MY_MQTT_CLIENT_PUBLISH_RETAIN

/* COMMAND MODE (NodeCommand.h - MyController API) */
#  define CMD_GROUP_HOST "192.168.0.105"
#  define CMD_GROUP_PORT 8081
#  define CMD_GROUP_AUTH "Base64 'login:password' string"
#  define CMD_GROUP_URI_ON "/mc/rest/resources/group/on"
#  define CMD_GROUP_URI_OFF "/mc/rest/resources/group/off"

/* INCLUSION BUTTON, LEDS */
#  define MY_INCLUSION_BUTTON_FEATURE 1

///
#  define INTERNAL_RF433_PIN WD0 // GPIO 16
#  define INTERNAL_LIVE_IR_RECEIVE_PIN WD3 // GPIO 0
#  define INTERNAL_LIVE_IR_SEND_PIN -1
///

#  define I2C_BMP180_ENABLE 1
#  define I2C_PCF8574_ENABLE 1
//#  define I2C_AP3216_ENABLE 1
//#  define INTERNAL_LIVE_VOLT_PIN A0  /* Internal power volts */
#  define INTERNAL_LIVE_ILLUMINATION_PIN A0 /* Internal illumination photo-resistor */

/* DEBUG */
#  define NO_DEBUG 1
#  define NO_DEBUG_RADIO 1
#  define LED_DEBUG 0          /* Atmega board internal LED Debug blink method */

#endif
