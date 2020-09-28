/**
 *  This file is available under the terms of the MIT License, see the LICENSE file for full details
 */

#if !defined(INT64STRING_H)
# define INT64STRING_H 1

# if (defined(ARDUINO) && (ARDUINO >= 100))
#  include "Arduino.h"
# else
#  include "WString.h"
# endif

String Int64ToString(uint64_t, uint8_t = DEC, bool = false, bool = false);
String Int64ToString(int64_t, uint8_t = DEC, bool = false);
uint64_t StringToInt64(String);
uint64_t StringToInt64(const char*, uint16_t);

#endif
