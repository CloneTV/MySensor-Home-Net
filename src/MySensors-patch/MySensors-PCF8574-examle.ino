
#define MY_DEFAULT_LED_I2C 1
#define MY_INCLUSION_MODE_FEATURE
#define MY_INCLUSION_BUTTON_FEATURE
#define MY_INCLUSION_MODE_DURATION 10

#include <MySensors.h>
#include <core/MyInclusionMode.h>
#include <PCF8574.h>

const uint8_t scl_PIN = D1;
const uint8_t sda_PIN = D2;
const uint8_t led_device[4] { 2U, 3U, 6U, 4U };
const uint8_t btn_device[4] { 0U, 1U, 5U, 7U };
PCF8574 expander(0x22, sda_PIN, scl_PIN);

void setup() {
  Wire.begin(sda_PIN, scl_PIN);
  Serial.begin(115200);
  while (!Serial) delay(50);
  
  for (uint8_t i = 0; i < 4U; i++) {
    expander.pinMode(led_device[i], OUTPUT);
    expander.digitalWrite(led_device[i], HIGH);
    Serial.printf("Init LED expander: %u/%u\n", (uint16_t)i, (uint16_t)led_device[i]);
  }
  for (uint8_t i = 0; i < 4U; i++) {
    expander.digitalWrite(btn_device[i], HIGH);
    expander.pinMode(btn_device[i], INPUT);
    Serial.printf("Init BTN expander: %u/%u\n", (uint16_t)i, (uint16_t)btn_device[i]);
  }
  expander.begin();
  ledsSetCb(
    [=](uint8_t & state) { expander.digitalWrite(led_device[1], state); },
    [=](uint8_t & state) { expander.digitalWrite(led_device[2], state); },
    [=](uint8_t & state) { expander.digitalWrite(led_device[3], state); }
  );
  inclusionSetCb(
    [=]() { return (expander.digitalRead(btn_device[0], true) == LOW); },
    [=](uint8_t & state) { expander.digitalWrite(led_device[0], state); }
  );
}

void loop() {}
