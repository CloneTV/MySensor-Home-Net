

#include "core/NodeOptions.h"
#include <ArduinoOTA.h>

mutex_t lockInit{};
uint8_t presentStatus = 1U;
uint16_t cnt = 0;
NodeLiveRssi   nlrssi = NodeLiveRssi();
NodeLiveMem    nlmem  = NodeLiveMem();
NodeI2CWeather nbaro  = NodeI2CWeather();
NodeI2CLight   nlight = NodeI2CLight();
NodeIrControl  nirrcv = NodeIrControl();

#if defined(MY_DEBUG)
const PROGMEM char * const str_ota[] = {
  "OTA Progress: %u%%\r", "Error[%u]: "
};
#endif

void before() {
  Wire.begin();
  INIT_LED();
  PRINTINIT();
  wait(500);

  PRINTLN("--- before START");
  CreateMutex(&lockInit);

  ArduinoOTA.setHostname(str_firmware[0]);
  ArduinoOTA.onStart([]() {
    PRINTLN("\nOTA start");
  });
  ArduinoOTA.onEnd([]() {
    PRINTLN("\nOTA end");
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

  wait(500);
}

void setup() {
  if (!GetMutex(&lockInit)) {
    PRINTLN("--- setup MUTEX LOCK");
    return;
  }
  do {
    if (!nbaro.init())
      break;
    if (!nlight.init())
      break;
    if (!nirrcv.init())
      break;
    if (!nlrssi.init())
      break;
    if (!nlmem.init())
      break;

    INFO_LED(1);
    ReleaseMutex(&lockInit);
    return;

  } while(false);

  PRINTLN("--- setup init sensors ERROR.. reboot");
  while(true) { PRINTLN("."); delay(500); }
}

bool presentationStep(uint8_t idx) {
  switch (idx) {
    case 1U: {
      if (!presentSend(str_firmware[0], str_firmware[1]))
        return false;
      break;
    }
    case 2U: {
      if (!nirrcv.presentation())
        return false;
      break;
    }
    case 3U: {
      if (!nbaro.getReady())
        return false;
      if (!nbaro.presentation())
        return false;
      break;
    }
    case 4U: {
      if (!nlight.presentation())
        return false;
      break;
    }
    case 5U: {
      if (!nlrssi.presentation())
        return false;
      break;
    }
    case 6U: {
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

  if (!GetMutex(&lockInit)) {
    PRINTLN("--- INIT presentation MUTEX LOCK");
    return;
  }
  if ((presentStatus == SENSOR_ID_NONE) || (presentStatus == 0U))
    return;
  
  while (presentStatus <= 6U) {
    if (!presentationStep(presentStatus)) {
      PRINTF("--- INIT presentation [%d] ERROR\n", presentStatus);
      ReleaseMutex(&lockInit);
      return;
    }
    PRINTF("--- INIT presentation [%d] OK\n", presentStatus);
    presentStatus++;
  }
  presentStatus = SENSOR_ID_NONE;
  ReleaseMutex(&lockInit);
  INFO_LED(2);
}
void loop() {
  if (presentStatus != SENSOR_ID_NONE) {
    presentation();
    if (presentStatus != SENSOR_ID_NONE)
      presentTimer<30>();

  } else if (isTransportReady()) {

    ArduinoOTA.handle();
    nirrcv.data(cnt);
    nlight.data(cnt);
    nbaro.data(cnt);
    nlmem.data(cnt);
    nlrssi.data(cnt);

    cnt++;
    if (cnt >= 60000U)
      cnt = 0U;
  } else {
    wait(10000);
  }
  wait(500);
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

  if (nirrcv.data(msg))
    return;
  if (nlight.data(msg))
    return;
  if (nbaro.data(msg))
    return;
  if (nlmem.data(msg))
    return;
  
  (void) nlrssi.data(msg);
}

// --------------------------------------------------------------------- //
