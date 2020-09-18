#if !defined(__MY_SENSOR_ACDCRELAY_H)
#define __MY_SENSOR_ACDCRELAY_H 1

/*
    EventSensor.n = sensor id
    EventSensor.p = sensor pin
    EventSensor.s = sensor state
    EventSensor.e = event enable/disable
 */
struct EventRelay {
  uint8_t n, p, s, e;
} __attribute__((packed));

/* ------- LIGHT ON/OFF SENSOR ------- */

class NodeRelay {
    private:
        bool isChange = false;
        NodeLiveLight const *light;

#  if defined(LIGHT_SENSOR)
#    if (LIGHT_SENSOR == 1)
        EventRelay ev[1] = {
          { 0U, 8U, 0U, 0U }
        };

#    elif (LIGHT_SENSOR == 2)
        EventRelay ev[2] = {
          { 0U, 8U, 0U, 0U },
          { 1U, 7U, 0U, 0U }
        };

#    elif (LIGHT_SENSOR == 3)
        EventRelay ev[3] = {
          { 0U, 8U, 0U, 0U },
          { 1U, 7U, 0U, 0U },
          { 2U, 4U, 0U, 0U }
        };

#    else
#       pragma message "WARNING - configuration don't use AC/DC relays, remove this include 'NodeRelay.h'"
        EventRelay ev[0]{};
#    endif
#  endif
    
    public:
        NodeRelay () {
        }
        NodeRelay(NodeLiveLight const *l) {
          light = l;
        }
        void init(uint16_t) {}
        void init() {

          MY_CRITICAL_SECTION {
            for (uint8_t i = 0U; i < __NELE(ev); i++) {

#               if (defined(LIGHT_ON_POWER) && (LIGHT_ON_POWER == ON))
                  ev[i].s = HIGH;
#               elif (defined(LIGHT_ON_POWER) && (LIGHT_ON_POWER == OFF))
                  ev[i].s = LOW;
#               else
                  ev[i].s = loadState(ev[i].n);
#               endif
                ev[i].e = ((ev[i].s) ? HIGH : LOW);

                pinMode(ev[i].p, OUTPUT);

                /*
                PRINTF("-- INIT RELAY [%u]: id=%u, pin=%u, state=%u, event=%u\n",
                    (uint16_t) (i + 1U), (uint16_t) ev[i].n, (uint16_t) ev[i].p,
                    (uint16_t) ev[i].s, (uint16_t) ev[i].e
                );
                */
            }
            isChange = true;
          }
        }
        bool presentation() {
          
          /*
          PRINTLN("NODE RELAY | presentation");
          */
          for (uint8_t i = 0U; i < __NELE(ev); i++) {

            if (!presentSend(ev[i].n, S_BINARY))
              return false;
            if (!presentSend(ev[i].n, V_STATUS))
              return false;
          }
          return true;
        }
        void data(uint16_t & cnt) {

          if (!isChange)
            return;
          isChange = false;

          /*
          PRINTLN("-- NODE RELAY | begin data");
          */
          for (uint8_t i = 0U; i < __NELE(ev); i++) {

            if (ev[i].e == LOW)
              continue;

            ev[i].e = LOW;
            digitalWrite(ev[i].p, ev[i].s);
            saveState(ev[i].n, ev[i].s);
            reportMsg(ev[i].n, V_STATUS, static_cast<bool>(ev[i].s));
                
            /*
            PRINTF("-- CHANGE RELAY | id=%u, pin=%u, state=%u, event=%u\n",
                (uint16_t) ev[i].n, (uint16_t) ev[i].p,
                (uint16_t) ev[i].s, (uint16_t) ev[i].e
            );
            */
            INFO_LED(i);
          }
        }
        bool data(const MyMessage & msg) {

          uint8_t idx;
          switch (msg.getType()) {
            case V_STATUS: {
              idx = sensorIsValid(ev, msg.sensor);
              if (idx == SENSOR_ID_NONE)
                return false;
              break;
            }
            default:
              return false;
          }
          
          /*
          PRINTF("-- INCOMING RELAY | begin MyMessage: %u, index=%u, type: %u\n",
              (uint16_t) msg.sensor, (uint16_t) idx, (uint16_t) msg.type
          );
          */

          switch (msg.getType()) {
            case V_STATUS: {
              if (ev[idx].s == msg.getByte())
                return true;

              MY_CRITICAL_SECTION {
                ev[idx].s = ((ev[idx].s > 0U) ? 0U : 1U);
                ev[idx].e = HIGH;
                isChange = true;
              }
              INFO_LED(ev[idx].n);
              break;
            }
        }
        return true;
      }
};

#endif

