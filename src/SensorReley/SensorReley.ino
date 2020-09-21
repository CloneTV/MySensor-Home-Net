
#include "core/NodeOptions.h"

uint8_t presentStatus = 1U;
uint16_t cnt = 0;
SensorInterface<NodeRelay>    nrelay = SensorInterface<NodeRelay>(new NodeRelay(clsLight));
SensorInterface<NodeLiveTemp> nltemp = SensorInterface<NodeLiveTemp>(new NodeLiveTemp());
SensorInterface<NodeLiveBat>  nlbat  = SensorInterface<NodeLiveBat>(new NodeLiveBat());
SensorInterface<NodeLiveRssi> nlrssi = SensorInterface<NodeLiveRssi>(new NodeLiveRssi());

void setup() {}
void before() {
  INIT_LED();
  PRINTINIT();
  PRINTBUILD();

  nltemp.init();
  nrelay.init();
  nlbat.init();
  nlrssi.init();
  clsLight->init();

  INFO_LED(1);
}
bool presentationStep(uint8_t idx) {
  switch (idx) {
    case 1U: {
      if (!presentSend(str_firmware[0], str_firmware[1]))
        return false;
      break;
    }
    case 2U: {
      if (!nltemp.presentation())
        return false;
      break;
    }
    case 3U: {
      if (!nrelay.presentation())
        return false;
      break;
    }
    case 4U: {
      if (!clsLight->presentation())
        return false;
      break;
    }
    case 5U: {
      if (!nlbat.presentation())
        return false;
      break;
    }
    case 6U: {
      if (!nlrssi.presentation())
        return false;
      break;
    }
    default: {
      PRINTLN("-- Wrong number ID\n");
      return false;
    }
  }
  return true;
}
void presentation() {
  if (presentStatus == SENSOR_ID_NONE)
    return;
  while (presentStatus <= 6U) {
    if (!presentationStep(presentStatus))
      return;
    presentStatus++;
  }
  presentStatus = SENSOR_ID_NONE;
  INFO_LED(2);
}
void loop() {
  if (presentStatus != SENSOR_ID_NONE) {
    presentation();
    if (presentStatus != SENSOR_ID_NONE)
      presentTimer<30>();

  } else if (isTransportReady()) {
    
    clsLight->data(cnt);
    nrelay.data(cnt);
    nltemp.data(cnt);
    nlbat.data(cnt);
    nlrssi.data(cnt);

    cnt++;
    if (cnt >= 60000U)
      cnt = 0U;
  } else {
    wait(10000);
  }
  wait(1000);
}
void receive(const MyMessage & msg) {
  if (msg.isAck()) {
    /*
     PRINTLN("GW | ACK");
     */
     return;
  }
  /*
  PRINTLN("GW | receive");
   */
  
  if (clsLight->data(msg))
    return;
  if (nrelay.data(msg))
    return;
  if (nltemp.data(msg))
    return;
  if (nlbat.data(msg))
    return;
  
  (void) nlrssi.data(msg);
}
