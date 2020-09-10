
#define NO_DEBUG 0
#define NO_DEBUG_RADIO 0
#define LED_DEBUG 1
#define LIGHT_SENSOR1 1

#include "sensorNode.h"
#include "AcDcRelay.h"
#include "NodeStat.h"

bool isRfGateway = false;
AcDcRelay relay = AcDcRelay();
NodeStat nstat = NodeStat();

void setup() {
  PRINTINIT();
  INFO_LED(1);
  transportSetNodeId();
}
void before() {
  pinMode(LED_BUILTIN, OUTPUT);
  nstat.init();
  relay.init(100);
  INFO_LED(2);
}
void presentation() {
  if (!(isRfGateway = presentSend(str_firmware[0], str_firmware[1])))
    return;
  if (!(isRfGateway = nstat.presentation()))
    return;

  isRfGateway = relay.presentation();
  PRINTF("-- Start: %s, radio=%d\n", str_firmware[0], isRfGateway);
  INFO_LED(3);
}
void loop() {
  if (!isRfGateway)
    presentation();

  relay.data();
  yield();
  nstat.data();
  wait(50);
}
void receive(const MyMessage & msg) {
  if (msg.isAck()) {
     PRINTLN("This is an ack from gateway");
  }
  relay.data(msg);
}
