
#include "core/NodeOptions.h"

mutex_t lockInit{};
uint8_t presentStatus = 1U;
uint16_t cnt = 0;

NodeLiveRssi    nlrssi = NodeLiveRssi();
NodeLiveMem     nlmem  = NodeLiveMem();
NodeLiveLight   nlight = NodeLiveLight();
NodeI2CWeather  nbaro  = NodeI2CWeather();
NodeI2CExpander nled   = NodeI2CExpander();
NodeCommand     ncmd   = NodeCommand();

void before() {
  Wire.begin(sda_PIN, scl_PIN);
  CreateMutex(&lockInit);
  OTASetup(
    str_firmware[0],
    static_cast<const char*>(MY_OTA_PASSWD)
  );
  PRINTLN("--- before() END");
}

void setup() {
  if (!GetMutex(&lockInit)) {
    PRINTLN("--- setup() MUTEX LOCK");
    return;
  }
  do {
    if (!nled.init())
      break;
    ledsSetCb(
      [=](uint8_t & state) { nled.myRxLed(state); },
      [=](uint8_t & state) { nled.myTxLed(state); },
      [=](uint8_t & state) { nled.myErrLed(state); }
    );
#   if defined(MY_INCLUSION_BUTTON_FEATURE)
    inclusionSetCb(
      [=]() { return nled.incluseBtn(); },
      [=](uint8_t & state) { nled.infoLed(state); }
    );
#   endif
    if (!nbaro.init())
      break;
    if (!nlight.init())
      break;
    if (!nlrssi.init())
      break;
    if (!nlmem.init())
      break;
    if (!ncmd.init())
      break;

#   if defined(ENABLE_I2C_SENSOR_ILLUMINATION)
    nlight.setCallBack(
      [=](float & level) {
        ncmd.lightReceive(level);
      }
    );
#   elif defined(ENABLE_LIVE_SENSOR_ILLUMINATION)
    nlight.setCallBack(
      [=](int16_t & level) {
        ncmd.lightReceive(level);
      }
    );
#   endif

    INFO_LED(1);
    ReleaseMutex(&lockInit);
    return;

  } while(false);

  PRINTLN("--- setup init sensors ERROR.. reboot");
  ERROR_LED(5000);
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
      if (!nlmem.presentation())
        return false;
      break;
    }
    case 3U: {
      if (!nlight.presentation())
        return false;
      break;
    }
    case 4U: {
      if (!nlrssi.presentation())
        return false;
      break;
    }
    case 5U: {
      if (!nled.presentation())
        return false;
      break;
    }
    case 6U: {
      if (!nbaro.getReady())
        return false;
      if (!nbaro.presentation())
        return false;
      break;
    }
    case 7U: {
      if (!ncmd.presentation())
        return false;
      break;
    }
    default: {
      PRINTLN("-- Wrong number ID\n");
      ERROR_LED(1000);
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
  
  while (presentStatus <= 7U) {
    if (!presentationStep(presentStatus)) {
      ERROR_LED(3000);
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
    PRINTLN("--- Presentation RETRY");
    presentation();
    if (presentStatus != SENSOR_ID_NONE)
      presentTimer<30>();

  } else if (isTransportReady()) {

    OTAHandler();
    nlight.data(cnt);
    ncmd.data(cnt);
    nbaro.data(cnt);
    nlmem.data(cnt);
    nlrssi.data(cnt);
    nled.data(cnt);

    cnt++;
    if (cnt >= 60000U)
      cnt = 0U;
  } else {
    ERROR_LED(3000);
    wait(10000);
  }
  wait(100);
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
  INFO_LED(1);

  if (nlight.data(msg))
    return;
  if (nbaro.data(msg))
    return;
  if (nlmem.data(msg))
    return;
  if (ncmd.data(msg))
    return;
  
  (void) nlrssi.data(msg);
}

// --------------------------------------------------------------------- //
