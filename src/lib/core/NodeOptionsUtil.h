#if !defined(__NODE_SENSORS_ALL_H)
#define __NODE_SENSORS_ALL_H 1

static inline bool presentData(const uint8_t id, const mysensors_sensor_t data, const char *desc) {
  wait(100);
  if (desc)
    return present(id, data, desc);
  else
    return present(id, data);
}
static inline bool presentData(const uint8_t id, const mysensors_data_t data, __attribute__ (( __unused__ )) const char*) {
  wait(100);
  return request(id, data);
}
static inline bool presentData(const char *name, const char *ver, __attribute__ (( __unused__ )) const char*) {
  wait(100);
  return sendSketchInfo(name, ver);
}

#define sensorIsValid(A, B) sensorIsValid_(A, __NELE(A), B);
template <typename T>
uint8_t sensorIsValid_(const T base[], const uint8_t & sz, const uint8_t & id) {
  for (uint8_t i = 0U; i < sz; i++) {
    if (id == base[i].n)
      return i;
  }
  return SENSOR_ID_NONE;
}

template<typename T>
void reportMsg(const uint8_t & id, const mysensors_data_t & tag, const T & val) {
    MyMessage msg(id, tag);
    send(msg.set(val), true);
}
template<typename T>
void reportMsg(const uint8_t & id, const mysensors_data_t & tag, const T & val, const uint8_t decimals) {
    MyMessage msg(id, tag);
    send(msg.set(val, decimals), true);
}
template<typename T>
void reportMsg(const uint8_t & id, const mysensors_internal_t & tag, const T & val) {
    _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, id, C_SET, tag, false).set(val));
}

template<typename T>
void dbgPrintWait(T const id) {
#   if (defined(MY_DEBUG) && !defined(MY_DISABLED_SERIAL))
      PRINT("-- present Send break, ");
      PRINTV(id);
      PRINT("\n");
#   endif
}

template<typename T1, typename T2>
bool presentSend(const T1 & id, const T2 & val, const char *desc = nullptr) {
  bool b = false;
  uint16_t cnt = 0;
  while (!b) {
    b = presentData(id, val, desc);
    if (++cnt > 1000) {
      dbgPrintWait(id);
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

#endif
