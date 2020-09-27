#if !defined(__MY_SENSOR_ACDCRELAY_H)
#define __MY_SENSOR_ACDCRELAY_H 1

# if defined(ENABLE_SENSOR_RELAY)

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

class NodeRelay : public SensorInterface<NodeRelay> {
    private:
        bool isChange = true;
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
        void AutoOff(uint8_t & idx) {
            if ((ev[idx].s == LOW) || (light == nullptr))
              return;
            
            switch(light->getState()) {
              case NodeLiveLight::LIGHTS::Bright:
              case NodeLiveLight::LIGHTS::High:
              case NodeLiveLight::LIGHTS::Light:
                break;
              default:
                return;
            }
            ev[idx].s = LOW;
            change(idx);
        }
        void change(uint8_t & idx) {
            digitalWrite(ev[idx].p, ev[idx].s);
            saveState(ev[idx].n, ev[idx].s);
            reportMsg(ev[idx].n, V_STATUS, static_cast<bool>(ev[idx].s));
        }
    
    public:
        NodeRelay () {
          light = nullptr;
        }
        NodeRelay(NodeLiveLight const *l) {
          light = l;
        }
        void go_init() {

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
          }
        }
        bool go_presentation() {
          
          /*
          PRINTLN("NODE RELAY | presentation");
          */
          for (uint8_t i = 0U; i < __NELE(ev); i++) {

            if (!presentSend(ev[i].n, S_BINARY, "Relay.Lights"))
              return false;
            if (!presentSend(ev[i].n, V_STATUS))
              return false;
          }
          return true;
        }
        void go_data(uint16_t & cnt) {

          if ((cnt % 70) == 0) {
            for (uint8_t i = 0U; i < __NELE(ev); i++)
              AutoOff(i);
          }

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
            change(i);
                
            /*
            PRINTF("-- CHANGE RELAY | id=%u, pin=%u, state=%u, event=%u\n",
                (uint16_t) ev[i].n, (uint16_t) ev[i].p,
                (uint16_t) ev[i].s, (uint16_t) ev[i].e
            );
            */
            INFO_LED(i);
          }
        }
        bool go_data(const MyMessage & msg) {

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

              ev[idx].s = msg.getByte();
              ev[idx].e = HIGH;
              isChange = true;
              INFO_LED(ev[idx].n);
              break;
            }
        }
        return true;
      }
};

# endif
#endif

