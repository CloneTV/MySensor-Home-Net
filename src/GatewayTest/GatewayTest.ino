
#include <Wire.h>
#include <ESP.h>

#  define PRINT(A) Serial.print(F(A))
#  define PRINTF(A, ...) Serial.printf(A, __VA_ARGS__); Serial.flush()
#  define PRINTV(A) Serial.print(A)
#  define PRINTVLN(A) Serial.println(A)
#  define PRINTLN(A) Serial.println(F(A)); Serial.flush()
#  define MY_DEBUG 1

#include "config.h"
#include "core/ESP8266_OTA.h"
#include "core/Int64String.h"

#define MY_DEFAULT_LED_EXT 1
#define MY_INCLUSION_MODE_FEATURE
#define MY_INCLUSION_BUTTON_FEATURE
#define MY_INCLUSION_MODE_DURATION 5

#include <MySensors.h>
#include <core/MyInclusionMode.h>
#include <PCF8574.h>

#define MY_RF433_ENABLE 1
#if defined(MY_RF433_ENABLE)
#  define RCSwitchDisableReceiving 1
#  define INTERNAL_RF433_GPIO 16
#  include <RCSwitch.h>
RCSwitch rfSwitch = RCSwitch();
#endif

#define MY_IR_ENABLE 1
#if defined(MY_IR_ENABLE)
#  include <IRremoteESP8266.h>
#  include <IRutils.h>
#  include <IRrecv.h>
#  define INTERNAL_LIVE_IR_RECEIVE 248
#  define INTERNAL_LIVE_IR_RECEIVE_PIN 0
IRrecv irRcv = IRrecv(INTERNAL_LIVE_IR_RECEIVE_PIN);
decode_results irdata{};
#endif


#define INFO_LED(A) __extension__ ({uint8_t cnt_ = A, b_ = LOW; while (--cnt_ > 0U) { infoLed(b_); b_ = ((b_ == LOW) ? HIGH : LOW); wait(500);};})
uint8_t state = LOW;

const PROGMEM char * const str_firmware[] = {
  MY_HOSTNAME, MY_VERSION
};
const uint8_t scl_PIN = WD1;
const uint8_t sda_PIN = WD2;
PCF8574 expander(0x22, sda_PIN, scl_PIN);
const uint8_t led_device[5] { 2U, 3U, 6U, 4U, 5U }; //2
const uint8_t btn_device[3] { 0U, 1U, 7U }; //0

// --------------------------------------------------------------------- //

void TestLed(uint8_t & state) {
  //expander.digitalWrite(led_device[0], ((state) ? HIGH : LOW));
  //Serial.printf("-- LED RX %u\n", state);
}
void infoLed(uint8_t & state) {
  expander.digitalWrite(led_device[0], state);
  //Serial.printf("-- LED RX %u\n", state);
}
void rxLed(uint8_t & state) {
  expander.digitalWrite(led_device[1], ((state) ? HIGH : LOW));
  //Serial.printf("-- LED RX %u\n", state);
}
void txLed(uint8_t & state) {
  expander.digitalWrite(led_device[2], ((state) ? HIGH : LOW));
  //Serial.printf("-- LED TX %u\n", state);
}
void errLed(uint8_t & state) {
  expander.digitalWrite(led_device[3], ((state) ? HIGH : LOW));
  //Serial.printf("-- LED ERR %u\n", state);
}
bool incluseBtn() {
  return (expander.digitalRead(btn_device[3], true) == LOW);
}

// ---------------------------------------------------------------------------- //
static inline bool presentData(const uint8_t id, const mysensors_sensor_t data, const char *desc) {
  if (desc)
    return present(id, data, desc);
  else
    return present(id, data);
}
static inline bool presentData(const uint8_t id, const mysensors_data_t data, __attribute__ (( __unused__ )) const char*) {
  return request(id, data);
}
static inline bool presentData(const char *name, const char *ver, __attribute__ (( __unused__ )) const char*) {
  return sendSketchInfo(name, ver);
}
// ---------------------------------------------------------------------------- //

void before() {
  Wire.begin(sda_PIN, scl_PIN);
  OTASetup(
    static_cast<const char*>(MY_HOSTNAME),
    static_cast<const char*>(MY_OTA_PASSWD)
  );
  Serial.println("before()");
  for (uint8_t i = 0; i < 5U; i++) {
    expander.pinMode(led_device[i], OUTPUT);
    expander.digitalWrite(led_device[i], HIGH);
    Serial.printf("Init LED expander: %u/%u\n", (uint16_t)i, (uint16_t)led_device[i]);
  }
  for (uint8_t i = 0; i < 3U; i++) {
    expander.pinMode(btn_device[i], INPUT);
    expander.digitalWrite(btn_device[i], HIGH);
    Serial.printf("Init BTN expander: %u/%u\n", (uint16_t)i, (uint16_t)btn_device[i]);
  }
  expander.begin();
  ledsSetCb(rxLed, txLed, errLed);
  inclusionSetCb(incluseBtn, TestLed);

#if defined(MY_RF433_ENABLE)
  rfSwitch.enableTransmit(INTERNAL_RF433_GPIO);
  //detachInterrupt(digitalPinToInterrupt(INTERNAL_RF433_GPIO));
#endif

#if defined(MY_IR_ENABLE)
  pinMode(INTERNAL_LIVE_IR_RECEIVE_PIN, INPUT); // INPUT_PULLUP
  irRcv.enableIRIn();
#endif

  (state == LOW) ? HIGH : LOW;
  infoLed(state);
}
void setup() {
  (state == LOW) ? HIGH : LOW;
  infoLed(state);
  Serial.println("setup()");
}
void presentation() {
  (state == LOW) ? HIGH : LOW;
  infoLed(state);
  Serial.println("presentation()");

  presentData(str_firmware[0], str_firmware[1], nullptr);
  
  for (uint8_t i = 1U; i < 6; i++) {
      presentData(i, S_BINARY, "Relay.Virtual.Lights");
      presentData(i, V_STATUS, nullptr);
  }

#if defined(MY_IR_ENABLE)
  presentData(INTERNAL_LIVE_IR_RECEIVE, S_IR, nullptr);
  presentData(INTERNAL_LIVE_IR_RECEIVE, V_IR_RECEIVE, nullptr);
#endif

}

void loop() {
  OTAHandler();
  
#if defined(MY_IR_ENABLE)
            if (irRcv.decode(&irdata)) {
                /// NEC only, ToDo: more standarts..
                if ((irdata.decode_type == NEC) && (irdata.bits == 32U) && (irdata.value != -1ULL)) {
                    String s = Int64ToString(irdata.value, HEX, true);
                    MyMessage msg((uint8_t)INTERNAL_LIVE_IR_RECEIVE, V_IR_RECEIVE);
                    send(msg.set(s.c_str()), true);
                    /*

                  Serial.print("-- irdata : ");
                  serialPrintUint64(irdata.value, HEX);
                  Serial.print(", ");
                  serialPrintUint64(irdata.value);
                  Serial.print("\n");
                  */
                }
                irRcv.resume();
            }
#endif
}

void receive(const MyMessage & msg) {
  (state == LOW) ? HIGH : LOW;
  infoLed(state);
  Serial.println("receive()");

  if (msg.getSender() != 0U)
    return;

#if defined(MY_RF433_ENABLE)
  switch (msg.getType()) {
    case V_STATUS: {
      Serial.printf("receive from: %u, sensor: %u\n", msg.getSender(), msg.sensor);
      switch(msg.sensor) {
        case 1: {
          //attachInterrupt(digitalPinToInterrupt(INTERNAL_RF433_GPIO), [](){}, CHANGE);
          rfSwitch.send((uint32_t)5180193, 24);
          //detachInterrupt(digitalPinToInterrupt(INTERNAL_RF433_GPIO));
          break;
        }
        case 2: {
          //attachInterrupt(digitalPinToInterrupt(INTERNAL_RF433_GPIO), [](){}, CHANGE);
          rfSwitch.send((uint32_t)5180194, 24);
          //detachInterrupt(digitalPinToInterrupt(INTERNAL_RF433_GPIO));
          break;
        }
      }
      break;
    }
  }
#endif
}
