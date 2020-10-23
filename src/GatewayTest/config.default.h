#if !defined(__MY_GW_CONFIG_ESP_H)
#define __MY_GW_CONFIG_ESP_H 1

#define MY_BAUD_RATE 115200

/* WI-FI */
#  define MY_WIFI_SSID "ssid"
#  define MY_WIFI_PASSWORD "pwd"
#  define MY_OTA_PASSWD "OTAPWD"
/* DEBUG */
// #  define NO_DEBUG
/* Firmware ID */
#  define MY_HOSTNAME "esp8266-Gateway"
#  define MY_VERSION "1.3"
/* MQTT */
#  define MY_GATEWAY_ESP8266
#  define MY_GATEWAY_MQTT_CLIENT
#  define MY_GATEWAY_MAX_CLIENTS 4
#  define MY_CONTROLLER_IP_ADDRESS 192, 168, 0, 105
#  define MY_MQTT_CLIENT_ID "group id"
#  define MY_MQTT_USER "user id"
#  define MY_MQTT_PASSWORD "user pwd"
#  define MY_MQTT_CLIENT_PUBLISH_RETAIN_SIGNED_FLAG 1
#  define MY_GATEWAY_MQTT_CLIENT
#  define MY_MQTT_PUBLISH_TOPIC_PREFIX "gw-out"
#  define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "gw-in"
#  define MY_PORT 1883
#  define MY_NODE_ID 0

#  define MY_DEBUG
#  define MY_RADIO_RF24
#  define MY_RF24_CE_PIN WD4
#  define MY_RF24_CS_PIN WD8
#  define MY_RF24_PA_LEVEL RF24_PA_HIGH
#  define MY_RF24_BASE_RADIO_ID 0x00,0xFC,0xE1,0xA8,0xA8
/*
 * #define MY_RF24_CHANNEL 76
 * 76 => 2476 Mhz (RF24 channel 77) standard
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
 */

#  define WD0 16  // Transmitter 433
#  define WD1 5   // SCL (I2C)
#  define WD2 4   // SDA (I2C)
#  define WD3 0   // -- (FLASH)
#  define WD4 2   // CE (SPI)
#  define WD5 14  // SCK (SPI)
#  define WD6 12  // MISO (SPI)
#  define WD7 13  // MOSI (SPI)
#  define WD8 15  // CSN (SPI)
#  define WTX 1   // -- (TX Serial)
#  define WRX 3   // -- (RX Serial)
#  define WA0 A0  // Lighting resistor

/* Wemos D1R1 */

#  if 0
#  define WD0 3   //
#  define WD1 1   //
#  define WD2 16  //
#  define WD3 5   // SCL (I2C) -- dup
#  define WD4 4   // SDA (I2C) -- dup
#  define WD5 14  // 
#  define WD6 12  // 
#  define WD7 13  // 
#  define WD8 0   //
#  define WD9 2   // CE (SPI)
#  define WD10 15 // CSN (SPI)
#  define WD11 13 // MOSI (SPI)
#  define WD12 12 // MISO (SPI)
#  define WD13 14 // SCK (SPI)
#  define WD14 4  // SDA (I2C) -- dup
#  define WD15 5  // SCL (I2C) -- dup
#  define WA0 A0  // Lighting resistor
#  endif

#endif
