
#define NO_DEBUG 0
#define NO_DEBUG_RADIO 1
#define LED_DEBUG 1
#define LIGHT_SENSOR1 1

#include "sensorNode.h"
#include "AcDcRelay.h"

bool isRfGateway = false;
AcDcRelay relay = AcDcRelay(); 

void setup() {
  PRINTINIT();
  INFO_LED(4);
}
void before() {
  pinMode(LED_BUILTIN, OUTPUT);
  relay.init(100);
  INFO_LED(2);
}
void presentation() {
  if (!(isRfGateway = presentSend(str_firmware[0], str_firmware[1])))
    return;
  
  isRfGateway = relay.presentation();
  PRINTF("-- Start: %s, radio=%d\n", str_firmware[0], isRfGateway);
  INFO_LED(3);
}
void loop() {
  if (!isRfGateway)
    presentation();

  relay.data();
  delay(50);
}
void receive(const MyMessage & msg) {
  if (msg.isAck()) {
     PRINTLN("This is an ack from gateway");
  }
  relay.data(msg);
}
