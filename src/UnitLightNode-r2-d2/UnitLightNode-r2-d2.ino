
#include "core/NodeOptions.h"

uint8_t presentStatus = 1U;
uint16_t cnt = 0;
NodeLiveTemp  nltemp = NodeLiveTemp();
NodeLiveBat   nlbat  = NodeLiveBat();
NodeLiveRssi  nlrssi = NodeLiveRssi();
NodeLiveMem   nlmem  = NodeLiveMem();
NodeLiveLight nlight = NodeLiveLight();
NodeRelay     nrelay = NodeRelay(&nlight);
NodeDimmer    ndimm  = NodeDimmer(&nlight);

void setup() {}
void before() {
  INIT_LED();
  PRINTINIT();
  PRINTBUILD();

  nlight.init();
  ndimm.init();
  nrelay.init();
  nltemp.init();
  nlbat.init();
  nlrssi.init();
  nlmem.init();
  
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
      if (!ndimm.presentation())
        return false;
      break;
    }
    case 3U: {
      if (!nrelay.presentation())
        return false;
      break;
    }
    case 4U: {
      if (!nlight.presentation())
        return false;
      break;
    }
    case 5U: {
      if (!nltemp.presentation())
        return false;
      break;
    }
    case 6U: {
      if (!nlbat.presentation())
        return false;
      break;
    }
    case 7U: {
      if (!nlrssi.presentation())
        return false;
      break;
    }
    case 8U: {
      if (!nlmem.presentation())
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
  while (presentStatus <= 8U) {
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
    
    nlight.data(cnt);
    ndimm.data(cnt);
    nrelay.data(cnt);
    nltemp.data(cnt);
    nlbat.data(cnt);
    nlrssi.data(cnt);
    nlmem.data(cnt);

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
  
  if (nlight.data(msg))
    return;
  if (nrelay.data(msg))
    return;
  if (ndimm.data(msg))
    return;
  if (nltemp.data(msg))
    return;
  if (nlbat.data(msg))
    return;
  if (nlmem.data(msg))
    return;
  
  (void) nlrssi.data(msg);
}
