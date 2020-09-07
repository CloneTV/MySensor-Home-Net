
#include <Arduino.h>

static inline bool presentData(const uint8_t id, const mysensors_sensor_t data) {
  return present(id, data);
}
static inline bool presentData(const uint8_t id, const mysensors_data_t data) {
  return request(id, data);
}
static inline bool presentData(const char *name, const char *ver) {
  return sendSketchInfo(name, ver);
}

template<typename T1, typename T2>
bool presentSend(const T1 id, const T2 val) {
  bool b = false;
  uint16_t cnt = 0;
  while (!b) {
    b = presentData(id, val);
    if (++cnt > 1000) {
#     if (defined(MY_DEBUG) && !defined(MY_DISABLED_SERIAL))
      PRINT("-- present Send break, ");
      PRINTV(id);
      PRINT("\n");
#     endif
      ERROR_LED(2);
      return b;
    }
    yield();
  }
  return b;
}
