/**
 *  This file is available under the terms of the MIT License, see the LICENSE file for full details
 *  
 *  Unsigned integer function, it does the bulk of the work, returns a String
 *  Based on work done by Rob Tillaart on the print9 function from:
 *  https://forum.arduino.cc/index.php?topic=143584.msg1519824#msg1519824
 */

#include "core/Int64String.h"
#define base16char(A) ("0123456789ABCDEF"[A])

String Int64ToString(uint64_t value, uint8_t base, bool prefix, bool sign) {
  if (base < 2)
    base = 2;
  else if (base > 16)
    base = 16;

  uint8_t i = 64;
  char buffer[66] = {0};

  if (value == 0)
    buffer[i--] = '0';
  else {
    uint8_t base_multiplied = 3;
    uint16_t multiplier = base * base * base;

    if (base < 16) {
      multiplier *= base;
      base_multiplied++;
    }
    if (base < 8) {
      multiplier *= base;
      base_multiplied++;
    }
    while (value > multiplier) {
      uint64_t q = value / multiplier;
      uint16_t r = value - q * multiplier;

      for (uint8_t j = 0; j < base_multiplied; j++) {
        uint16_t rq = r / base;
        buffer[i--] = base16char(r - rq * base);
        r = rq;
      }
      value = q;
    }

    uint16_t remaining = value;
    while (remaining > 0) {
      uint16_t q = remaining / base;
      buffer[i--] = base16char(remaining - q * base);
      remaining = q;
    }
  }

  if (base == DEC && sign)
    buffer[i--] = '-';
  else if (prefix) {
    if (base == HEX) {
      buffer[i--] = 'x';
      buffer[i--] = '0';
    }
    else if (base == OCT)
      buffer[i--] = '0';
    else if (base == BIN)
      buffer[i--] = 'B';
  }
  return String(&buffer[i + 1]);
}

String Int64ToString(int64_t value, uint8_t base, bool prefix) {
  bool sign = base == DEC && value < 0;
  uint64_t uvalue = sign ? -value : value;
  return Int64ToString(uvalue, base, prefix, sign);
}

uint64_t StringToInt64(String str) {
  return StringToInt64(str.c_str(), str.length());
}

uint64_t StringToInt64(const char *s, uint16_t sz = 0U) {
  uint64_t val = 0ULL;
  if (s == nullptr)
    return val;

  if (sz <= 0)
    sz = strlen(s);

  if (sz <= 2)
    return val;
  
  uint16_t i = (((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X'))) ? 2U : 0U);
  for (; i < sz; i++) {
    const char c = s[i];
    if (!isxdigit(c))
      return -1ULL;
    
    val *= 16;
    val += (c >= '0' && c <= '9') ? c - '0' : c - 'A' + 10;
  }
  return val;
}
