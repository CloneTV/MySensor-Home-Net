
#define MY_DEBUG
// #define MY_SPECIAL_DEBUG
// #define MY_DEBUG_VERBOSE
// #define MY_DEBUG_VERBOSE_RF24
#define MY_DEBUG_VERBOSE_SIGNING

#define MY_RADIO_RF24
#define MY_RF24_CE_PIN D2
#define MY_RF24_CS_PIN D4
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
//#define MY_RF24_ENABLE_ENCRYPTION

#define MY_BAUD_RATE 115200
#define MY_NODE_ID 22
// #define MY_NODE_ID (AUTO)
//#define MY_PARENT_NODE_ID (AUTO)
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC

#define MY_HOSTNAME "sensor-Fake"
#include <MySensors.h>

#define OPEN 1
#define CLOSE 0
#define NODE_SENSOR_ID 1

MyMessage msg(NODE_SENSOR_ID, V_TRIPPED);

int cnt = 0;
uint8_t value = OPEN;

void setup() {}
void presentation()
{
  sendSketchInfo("Fake Sensor", "1.1");
  present(NODE_SENSOR_ID, S_DOOR);
}

void loop()
{
  if (++cnt >= 60 ) {
    value = ((value == OPEN) ? CLOSE : OPEN);
    send(msg.set(value));
    Serial.printf("Set DOOR status = %d\n", (int) value);
    cnt = 0;
  }
  delay(100);
}
