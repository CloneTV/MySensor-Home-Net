
/*
Скетч использует 407300 байт (38%) памяти устройства. Всего доступно 1044464 байт.
Глобальные переменные используют 31716 байт (38%) динамической памяти, оставляя 50204 байт для локальных переменных. Максимум: 81920 байт.
 */

#include "core/NodeOptions.h"

bool isConnect = true;
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
  if (!nled.init()) {
    PRINTLN("--- setup I2C leds ERROR.. reboot");
    while(true) { ERROR_LED(500); }
  }
  ledsSetCb(
      [=](uint8_t & state) { nled.myRxLed(state); },
      [=](uint8_t & state) { nled.myTxLed(state); },
      [=](uint8_t & state) { nled.myErrLed(state); }
  );
# if defined(MY_INCLUSION_BUTTON_FEATURE)
  inclusionSetCb(
      [=]() { return nled.incluseBtn(); },
      [=](uint8_t & state) { nled.infoLed(state); }
  );
# endif
  INFO_LED(1);
  OTASetup(
    str_firmware[0],
    static_cast<const char*>(MY_OTA_PASSWD),
    [=](uint8_t val) { nled.errorLed(val); }
  );
  PRINTLN("--- before() END");
}

void setup() {
  if (!GetMutex(&lockInit)) {
    PRINTLN("--- setup() MUTEX LOCK");
    return;
  }
  do {
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

    INFO_LED(2);
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
  INFO_LED(3);
}
void loop() {
  if (presentStatus != SENSOR_ID_NONE) {
    PRINTLN("--- Presentation RETRY");
    presentation();
    if (presentStatus != SENSOR_ID_NONE)
      presentTimer<30>();

  } else if (isTransportReady()) {

    do {
      if (WiFi.status() != WL_CONNECTED) {
        isConnect = false;
        break;
      }
      
      OTAHandler();
      nlight.data(cnt);
      nbaro.data(cnt);
      nlmem.data(cnt);
      nlrssi.data(cnt);
      nled.data(cnt);

    } while (0);

    ncmd.data(cnt);

    if ((WiFi.status() == WL_CONNECTED) && (!isConnect)) {
      isConnect = true;
      OTAReInit(
        str_firmware[0],
        [=](uint8_t val) { nled.errorLed(val); }
      );
    }

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
#  if defined(I2C_PCF8574_ENABLE)
   uint8_t busy = HIGH;
   nled.infoLed(busy);
#  endif

  do {
    if (nlight.data(msg))
      break;
    if (nbaro.data(msg))
      break;
    if (nlmem.data(msg))
      break;
    if (ncmd.data(msg))
      break;
    if (nlrssi.data(msg))
      break;
  } while (false);

#  if defined(I2C_PCF8574_ENABLE)
   busy = LOW;
   nled.infoLed(busy);
#  endif
}

// --------------------------------------------------------------------- //
