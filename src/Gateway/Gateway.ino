/*
 *
 * DESCRIPTION
 * The ESP8266 MQTT gateway sends radio network (or locally attached sensors) data to your MQTT broker.
 * The node also listens to MY_MQTT_TOPIC_PREFIX and sends out those messages to the radio network
 *
 * LED purposes:
 * - To use the feature, uncomment any of the MY_DEFAULT_xx_LED_PINs in your sketch
 * - RX (green) - blink fast on radio message received. In inclusion mode will blink fast only on presentation received
 * - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
 * - ERR (red) - fast blink on error during transmission error or receive crc error
 *
 * See https://www.mysensors.org/build/connect_radio for wiring instructions.
 *
 * If you are using a "barebone" ESP8266, see
 * https://www.mysensors.org/build/esp8266_gateway#wiring-for-barebone-esp8266
 *
 * Inclusion mode button:
 * - Connect GPIO5 (=D1) via switch to GND ('inclusion switch')
 *
 */

#include "Gateway.h"
#include <ArduinoOTA.h>
#include <MySensors.h>
#include "IrControl.h"
#include "WeatherControl.h"

const PROGMEM char *str_firmware[] = {
  MY_HOSTNAME, MY_VERSION
};

#if defined(MY_DEBUG)
const PROGMEM char *str_ota[] = {
  "OTA Progress: %u%%\r", "Error[%u]: "
};
#endif

IrControl ir{};
WeatherControl weather{};

template<typename T1, typename T2>
void presentSend(const T1 id, const T2 val) {
  bool b = false;
  uint32_t cnt = 0;
  while (!b) {
    b = presentData(id, val);
    if (++cnt > 1000) {
      PRINT("-- present Send break, ");
      PRINTV(id);
      PRINT("\n");
      ERROR_LED();
      break;
    }
  }
}

void setup() {

#if defined(MY_DEFAULT_ERR_LED_PIN)
  pinMode(MY_DEFAULT_ERR_LED_PIN, OUTPUT);
  digitalWrite(MY_DEFAULT_ERR_LED_PIN, LOW);
#endif
  
  pinMode(PIN_SENSOR_IR_SEND, OUTPUT);
  pinMode(PIN_SENSOR_IR_RECV, INPUT);
  digitalWrite(PIN_SENSOR_IR_SEND, LOW);
  digitalWrite(PIN_SENSOR_IR_RECV, LOW);

  ArduinoOTA.setHostname(str_firmware[0]);
  ArduinoOTA.onStart([]() {
    PRINTLN("ArduinoOTA start");
  });
  ArduinoOTA.onEnd([]() {
    PRINTLN("\nArduinoOTA end\n");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    PRINTF(str_ota[0], (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    PRINTF(str_ota[1], error);
    if (error == OTA_AUTH_ERROR) {
      PRINTLN("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      PRINTLN("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      PRINTLN("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      PRINTLN("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      PRINTLN("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void before() {
# if (defined(MY_DEBUG) && !defined(MY_DISABLED_SERIAL))
# endif
  ir.init();
  if (!weather.init(BMP_ADDRESS)) {
    PRINTLN("BMP280 sensor detect failed!");
    ERROR_LED();
    for (;;);
  }
}

void presentation() {
  
  presentSend(str_firmware[0], str_firmware[1]);
  //
  presentSend(NODE_SENSOR_IR_SEND, S_IR);
  presentSend(NODE_SENSOR_IR_SEND, V_IR_SEND);
  presentSend(NODE_SENSOR_IR_RECV, S_IR);
  presentSend(NODE_SENSOR_IR_RECV, V_IR_RECEIVE);
  //
  presentSend(NODE_SENSOR_WETHER_TEMP, S_TEMP);
  presentSend(NODE_SENSOR_WETHER_BARO, S_BARO);
}

void loop() {
  ArduinoOTA.handle(); yield();
  ir.data();           yield();
  weather.data();      yield();
  delay(100);
  
}

void receive(const MyMessage &msg) {
    if (msg.isAck())
      PRINTLN("-- !! This is an ack from gateway..");
    if (msg.type == V_IR_SEND)
      ir.data(msg);
}

bool presentData(const uint8_t id, const mysensors_sensor_t data) {
  return present(id, data);
}
bool presentData(const uint8_t id, const mysensors_data_t data) {
  return request(id, data);
}
bool presentData(const char *name, const char *ver) {
  return sendSketchInfo(name, ver);
}
