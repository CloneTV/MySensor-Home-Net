
#include "core/SensorNode.h"
#include "core/Presenter.h"
#include "core/NodeStat.h"
#include "core/AcDcRelay.h"

bool isRfGateway = false;
AcDcRelay relay = AcDcRelay();
NodeStat nstat = NodeStat();

void setup() {
  PRINTINIT();
  INFO_LED(1);
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
  PRINTF("-- Start: %s, relay=%d\n", str_firmware[0], isRfGateway);
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
     PRINTLN("GW | ACK");
  }
  if (relay.data(msg))
    return;
  (void) nstat.data(msg);
}
