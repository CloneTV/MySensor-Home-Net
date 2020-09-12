
#include "core/SensorNode.h"
#include "core/Presenter.h"
#include "core/NodeStat.h"
#include "core/PwmHw.h"
#include "core/Dimmer.h"
#include "core/AcDcRelay.h"

bool isRfGateway = false;
AcDcRelay nrelay = AcDcRelay();
Dimmer    ndim = Dimmer();
NodeStat  nstat = NodeStat();

void setup() {
  PRINTINIT();
  INFO_LED(1);
  PRINTF("-- Build: %s - %s, %s [%s:%s]\n",
    str_build[0], str_build[1], str_build[2],
    str_firmware[0], str_firmware[1]
  );
}
void before() {
  INIT_LED();
  nstat.init();
  ndim.init();
  nrelay.init(100);
  INFO_LED(2);
}
void presentation() {
  if (!(isRfGateway = presentSend(str_firmware[0], str_firmware[1])))
    return;
  if (!(isRfGateway = nstat.presentation()))
    return;
  if (!(isRfGateway = nrelay.presentation()))
    return;
  if (!(isRfGateway = ndim.presentation()))
    return;

  PRINTF("-- Start: %s, relays=%u, dimmers=%u\n",
    str_firmware[0], size_light_sensors, size_dimmer_sensors
  );
  INFO_LED(3);
}
void loop() {
  if (!isRfGateway)
    presentation();
  if (isRfGateway) {
    ndim.data();
    yield();
    nrelay.data();
    yield();
    nstat.data();
  }
  wait(50);
}
void receive(const MyMessage & msg) {
  if (msg.isAck()) {
     PRINTLN("GW | ACK");
  }
  if (nrelay.data(msg))
    return;
  if (ndim.data(msg))
    return;
  (void) nstat.data(msg);
}
