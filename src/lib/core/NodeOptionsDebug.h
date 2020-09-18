#if !defined(__MY_SENSOR_OPT_DEBUG_H)
#define __MY_SENSOR_OPT_DEBUG_H 1

#   if defined(MY_DEBUG)
#     if defined(ENABLE_LIVE_SENSOR_ILLUMINATION)
#       pragma message "INTERNAL (ILLUMINATION [fotoresistor]) COMPILED! SENSOR ID=251"
#     endif
#     if defined(ENABLE_LIVE_SENSOR_VOLT)
#       pragma message "INTERNAL (VOLT) COMPILED! SENSOR ID=252"
#     endif
#     if defined(ENABLE_LIVE_SENSOR_TEMP)
#       pragma message "INTERNAL (TEMP) COMPILED! SENSOR ID=253"
#     endif
#     if defined(ENABLE_LIVE_SENSOR_RSSI)
#       pragma message "INTERNAL (RSSI) COMPILED! SENSOR ID=254"
#     endif

#     if defined(LIGHT_SENSOR)
#       if (LIGHT_SENSOR == 1)
#          pragma message "LIGHT SENSORS NUMBER=1, BTN PIN=4, RELEY PIN=7"
#       elif (LIGHT_SENSOR == 2)
#          pragma message "LIGHT SENSORS NUMBER=2, BTN PIN=4,2, RELEY PIN=7,8"
#       elif (LIGHT_SENSOR == 3)
#          pragma message "LIGHT SENSORS NUMBER=3, BTN PIN=4,5,6 RELEY PIN=7,8,3"
#       endif
#     endif
#     if defined(DIMMER_SENSOR)
#       if (DIMMER_SENSOR == 1)
#          pragma message "DIMMER SENSORS NUMBER=1, DIMMER PIN=5"
#       elif (DIMMER_SENSOR == 2)
#          pragma message "DIMMER SENSORS NUMBER=2, DIMMER PIN=5,6"
#       elif (DIMMER_SENSOR == 3)
#          pragma message "DIMMER SENSORS NUMBER=3, DIMMER PIN=5,6,3"
#       endif
#     endif
#   endif

#endif
