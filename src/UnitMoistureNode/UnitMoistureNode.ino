/*
Скетч использует 9644 байт (31%) памяти устройства. Всего доступно 30720 байт.
Глобальные переменные используют 339 байт (16%) динамической памяти, оставляя 1709 байт для локальных переменных. Максимум: 2048 байт.
*/

#include "core/NodeOptions.h"

uint8_t presentStatus = 1U;
NodeLiveTemp    nltemp = NodeLiveTemp();
NodeMoisture    nsoil  = NodeMoisture();

void setup() {}
void before() {
  INIT_LED();
  INIT_PRINT();
  BUILD_PRINT();

  if (!nsoil.init()) {
    while (1) {
      ERROR_LED(1000);
    }
  }
  nltemp.init();
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
      if (!nsoil.presentation())
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

  if (!nsoil.rfbegin()) {
    return;
  }
  while (presentStatus <= 3U) {
    if (!presentationStep(presentStatus)) {
      nsoil.rfend();
      return;
    }
    presentStatus++;
  }
  presentStatus = SENSOR_ID_NONE;
  nsoil.rfend();
  PRINTLN("--- presentation() - END OK");
}
void loop() {
  if (presentStatus != SENSOR_ID_NONE) {
    presentation();
    if (presentStatus != SENSOR_ID_NONE)
      sleep(MY_TIME_SLEEPS / 10);

  } else {
    
    do {
      uint16_t cnt = 0U;

      nsoil.enable();
      nsoil.data(cnt);
      if (!nsoil.ischange())
        break;

      //
      nltemp.enable();
      nltemp.data(cnt);

    } while (0);

    nsoil.rfend();

  }
  PRINTLN("--- sleep begin()");
  sleep(digitalPinToInterrupt(MY_PASSIVE_BTN_PIN), RISING, MY_TIME_SLEEPS, false);
  PRINTLN("--- sleep end()");
}
void receive(const MyMessage & msg) {
  if (msg.isAck()) {
     return;
  }
  if (nltemp.data(msg))
    return;
  
  (void) nsoil.data(msg);
}
