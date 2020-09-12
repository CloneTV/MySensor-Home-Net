#if !defined(__MY_SENSOR_PWM_H)
#define __MY_SENSOR_PWM_H 1

#if !defined(SHIM_PIN)
#  if !defined(SLED_PIN)
#    define SHIM_PIN SLED_PIN
#  endif
#endif

#if (defined(SHIM_PIN) && ((SHIM_PIN == 5) || (SHIM_PIN == 6)))
#  define initPwmHw() \
     { pinMode(SHIM_PIN, OUTPUT); TCCR0A = 0x23; TCCR0B = 0x09; OCR0A = 159; }
#elif (defined(SHIM_PIN) && ((SHIM_PIN == 3) || (SHIM_PIN == 11)))
#  define initPwmHw() \
     { pinMode(SHIM_PIN, OUTPUT); TCCR1A = 0x23; TCCR1B = 0x09; OCR1A = 159; }
#elif (defined(SHIM_PIN) && ((SHIM_PIN == 9) || (SHIM_PIN == 10)))
#  define initPwmHw() \
     { pinMode(SHIM_PIN, OUTPUT); TCCR2A = 0x23; TCCR2B = 0x09; OCR2A = 159; }
#  else
#    define initPwmHw()
#endif

static inline void initPwm(int16_t pin) {
  switch (pin) {
    case 3:
    case 11: {
      TCCR2A = 0x23;
      TCCR2B = 0x09;
      OCR2A = 159;
      break;
    }
    case 6:
    case 5: {
      TCCR0A = 0x23;
      TCCR0B = 0x09;
      OCR0A = 159;
      break;
    }
    case 9:
    case 10: {
      TCCR1A = 0x23;
      TCCR1B = 0x09;
      OCR1A = 159;
      break;
    }
  }
}

static inline void setPwm(int16_t pin, int16_t d) {
  byte mode;
  bool ext_pin = (pin == 3 || pin == 11);
  switch(d) {
    case 1: {
      mode = 0x01;
      break;
    }
    case 8: {
      mode = 0x02;
      break;
    }
    case 32: {
      mode = (ext_pin) ? 0x03 : 0;
      break;
    }
    case 64: {
      mode = (ext_pin) ? 0x04 : 0x03;
      break;
    }
    case 128: {
      mode = (ext_pin) ? 0x05 : 0;
      break;
    }
    case 256: {
      mode = (ext_pin) ? 0x06 : 0x04;
      break;
    }
    case 1024: {
      mode = (ext_pin) ? 0x07 : 0x05;
      break;
    }
    default: return;
  }
  
  if (!mode)
    return;
    
  switch (pin) {
    case 3:
    case 11: {
      TCCR2B = TCCR2B & 0b11111000 | mode;
      break;
    }
    case 6:
    case 5: {
      TCCR0B = TCCR0B & 0b11111000 | mode;
      break;
    }
    case 9:
    case 10: {
      TCCR1B = TCCR1B & 0b11111000 | mode;
      break;
    }
  }
}

static inline void setupPwm(int16_t pin) {
  switch (pin) {
    case 6:
    case 5: {
      setPwm(pin, 8);
      break;
    }
  }
}

#endif

