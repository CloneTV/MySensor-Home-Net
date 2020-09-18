#if !defined(__NODE_SENSORS_ALL_H)
#define __NODE_SENSORS_ALL_H 1

#define sensorIsValid(A, B) sensorIsValid_(A, __NELE(A), B);
template <typename T>
uint8_t sensorIsValid_(const T base[], const uint8_t & sz, const uint8_t & id) {
  for (uint8_t i = 0U; i < sz; i++) {
    if (id == base[i].n)
      return i;
  }
  return SENSOR_ID_NONE;
}

template <typename T>
void reportMsg(const uint8_t & id, const mysensors_data_t & tag, const T & val) {
    MyMessage msg(id, tag);
    send(msg.set(val), true);
}

template<typename T1, typename T2>
bool presentSend(const T1 & id, const T2 & val) {
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

template <uint8_t T>
void presentTimer() {
  uint16_t cnt = 0U,
           cmp = (T * 2);
  while (++cnt < cmp) {
    PRINT(".");
    wait(500);
    yield();
  }
  PRINTLN(" - timer end");
}

static inline bool presentData(const uint8_t id, const mysensors_sensor_t data) {
  return present(id, data);
}
static inline bool presentData(const uint8_t id, const mysensors_data_t data) {
  return request(id, data);
}
static inline bool presentData(const char *name, const char *ver) {
  return sendSketchInfo(name, ver);
}

#endif
