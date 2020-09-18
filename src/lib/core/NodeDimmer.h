#if !defined(__MY_SENSOR_DIMMER_H)
#define __MY_SENSOR_DIMMER_H 1

#  if !defined(LIGHT_SENSOR)
#    define LIGHT_SENSOR 0
#  endif
#  define FADE_DELAY 10

/*
    EventSensor.n = sensor id
    EventSensor.p = sensor pin
    EventSensor.s = sensor state
    EventSensor.o = sensor old state
    EventSensor.e = event enable/disable
    EventSensor.val = incoming procentage value
    EventSensor.old = old procentage value
 */
typedef struct _EventDimmer {
  uint8_t n, p, s, e, o;
  int16_t val, old;
} EventDimmer __attribute__((packed));

/* ------- DIMMER_SENSOR ------- */

class NodeDimmer {

    private:
        bool isChange = false;
        NodeLiveLight const *light;

#  if defined(DIMMER_SENSOR)
#    if (DIMMER_SENSOR == 1)
        EventDimmer ev[1] = {
          { (LIGHT_SENSOR + 1), 5U, 0U, 0U, 0U, 0U, 0U }
        };

#    elif (DIMMER_SENSOR == 2)
        EventDimmer ev[2] = {
          { (LIGHT_SENSOR + 1), 5U, 0U, 0U, 0U, 0U, 0U },
          { (LIGHT_SENSOR + 2), 6U, 0U, 0U, 0U, 0U, 0U }
        };

#    elif (DIMMER_SENSOR == 3)
        EventDimmer ev[3] = {
          { (LIGHT_SENSOR + 1), 5U, 0U, 0U, 0U, 0U, 0U },
          { (LIGHT_SENSOR + 2), 6U, 0U, 0U, 0U, 0U, 0U },
          { (LIGHT_SENSOR + 3), 3U, 0U, 0U, 0U, 0U, 0U }
        };

#    else
#       pragma message "WARNING - configuration don't use dimmers, remove this include 'NodeDimmer.h'"
        EventDimmer ev[0]{};
#    endif
#  endif

        void fade(uint8_t idx) {
            int8_t d = (((ev[idx].val - ev[idx].old) < 0) ? -1 : 1);
            while (ev[idx].old != ev[idx].val) {
                ev[idx].old += d;
                analogWrite(
                  ev[idx].p,
                  static_cast<int32_t>((ev[idx].old / 100.) * 255)
                );
                wait(FADE_DELAY);
                if ((ev[idx].old >= 100) || (ev[idx].old <= 0))
                  break;
            }
        }
        void change(uint8_t idx) {
            fade(idx);
            reportMsg(ev[idx].n, V_STATUS, static_cast<bool>(ev[idx].s));
            reportMsg(ev[idx].n, V_PERCENTAGE, ev[idx].val);
        }
   
    public:
        NodeDimmer() {
        }
        NodeDimmer(NodeLiveLight const *l) {
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

                ev[i].o = 0U;
                ev[i].e = ((ev[i].s) ? HIGH : LOW);
                ev[i].val = ((ev[i].s) ? 100 : 0);
                ev[i].old = 0;

                pinMode(ev[i].p, OUTPUT);
                setupPwm(ev[i].p);

                /*
                PRINTF("-- INIT DIMMER [%u] | id=%u, pin=%u, state=%u/%u, event=%u, level=%u\n",
                    (uint16_t) (i + 1U), (uint16_t) ev[i].n, (uint16_t) ev[i].p,
                    (uint16_t) ev[i].s,(uint16_t) ev[i].o, (uint16_t) ev[i].e, ev[i].val
                );
                */
            }
            isChange = true;
          }
        }
        bool presentation() {

            /*
            PRINTLN("NODE DIMMER | presentation");
            */
            for (uint8_t i = 0U; i < __NELE(ev); i++) {
                
                if (!presentSend(ev[i].n, S_DIMMER))
                    return false;
                if (!presentSend(ev[i].n, V_STATUS))
                    return false;
                if (!presentSend(ev[i].n, V_PERCENTAGE))
                    return false;
            }
            return true;
        }
        void data(uint16_t & cnt) {

          if (!isChange)
            return;
          isChange = false;

          /*
          PRINTLN("-- NODE DIMMER | begin data");
          */
          for (uint8_t i = 0U; i < __NELE(ev); i++) {

            if (ev[i].e == LOW)
              continue;

            if (ev[i].s != ev[i].o) {
              ev[i].o = ev[i].s;

              if (ev[i].s == HIGH)
                ev[i].val = ((ev[i].val > 0) ? ev[i].val : 100);
              else
                ev[i].val = 0;
              change(i);

            } else if ((ev[i].s == LOW) && (ev[i].val > 0)) {
              ev[i].o = ev[i].s = HIGH;
              change(i);

            } else if ((ev[i].s == HIGH) && (ev[i].val == 0)) {
              ev[i].o = ev[i].s = LOW;
              change(i);

            } else {
              fade(i);
              reportMsg(ev[i].n, V_PERCENTAGE, ev[i].val);
              
            }
            ev[i].e = LOW;
            wait(50);
            
            /*
            PRINTF("-- CHANGE DIMMER | id=%u, pin=%u, state=%u/%u, event=%u, level=%u\n",
                (uint16_t) ev[i].n, (uint16_t) ev[i].p, (uint16_t) ev[i].s,
                (uint16_t) ev[i].o, (uint16_t) ev[i].e, ev[i].val
            );
            */
            INFO_LED(i);
          }
        }
        bool data(const MyMessage & msg) {

            uint8_t idx;
            switch (msg.getType()) {
                case V_STATUS:
                case V_PERCENTAGE: {
                  idx = sensorIsValid(ev, msg.sensor);
                  if (idx == SENSOR_ID_NONE)
                    return false;
                  break;
                }
                default:
                    return false;
            }

            /*
            PRINTF("-- INCOMING DIMMER | begin MyMessage: %u, index=%u, type: %u\n",
                (uint16_t) msg.sensor, (uint16_t) idx, (uint16_t) msg.type
            );
            */

            switch (msg.getType()) {
                case V_STATUS: {
                  ev[idx].s = msg.getByte();
                  if (ev[idx].s == ev[idx].o)
                    return true;
                  break;
                }
                case V_PERCENTAGE: {

                  MY_CRITICAL_SECTION {
                    int16_t val = msg.getInt();
                    val = ((val > 100) ? 100 : ((val < 0) ? 0 : val));

                    if (ev[idx].val == val)
                      return true;
                  
                    ev[idx].s = ((val > 0) ? HIGH : LOW);
                    ev[idx].val = val;
                  }
                  break;
                }
            }
            MY_CRITICAL_SECTION {
              ev[idx].e = HIGH;
              isChange = true;
            }
            INFO_LED(ev[id].n);
            return true;
        }
};

#endif
