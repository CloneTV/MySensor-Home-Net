#if !defined(__MY_SENSOR_ACDCRELAY_BUTTON_H)
#define __MY_SENSOR_ACDCRELAY_BUTTON_H 1

# if defined(ENABLE_SENSOR_RELAY_BTN)

#  include <Bounce2.h>
#  define BOUNCER_INTERVAL 100

/*
    EventSensor.n = sensor id
    EventSensor.p = sensor pin
    EventSensor.s = sensor state
    EventSensor.e = event enable/disable
 */
struct EventRelayButton {
  uint8_t n, pr, pb, s, e;
  Bounce *b;
} __attribute__((packed));

/* ------- LIGHT BUTTON ON/OFF SENSOR ------- */

#  if (defined(DIMMER_SENSOR) && (DIMMER_SENSOR > 0))
#    pragma message "WARNING - you configuration don't support 'AC/DC relays buttons' AND 'Dimmers', pins intersect! Disable either one or the other, or edit the pin tables"
#  endif 

#  if (defined(LIGHT_SENSOR) && (LIGHT_SENSOR > 0))
#    pragma message "WARNING - you configuration don't support 'AC/DC relays buttons' AND 'AC/DC relays', pins intersect! Disable either one or the other, or edit the pin tables"
#  endif 

class NodeRelayButton {
    private:
        bool isChange = true;
        NodeLiveLight const *light;

#  if defined(LIGHT_SENSOR_BTN)
#    if (LIGHT_SENSOR_BTN == 1)
        EventRelayButton ev[1] = {
          { 0U, 8U, 5U, 0U, 0U }
        };

#    elif (LIGHT_SENSOR_BTN == 2)
        EventRelayButton ev[2] = {
          { 0U, 8U, 5U, 0U, 0U },
          { 1U, 7U, 6U, 0U, 0U }
        };

#    elif (LIGHT_SENSOR_BTN == 3)
        EventRelayButton ev[3] = {
          { 0U, 8U, 5U, 0U, 0U },
          { 1U, 7U, 6U, 0U, 0U },
          { 2U, 4U, 3U, 0U, 0U }
        };

#    else
#       pragma message "WARNING - configuration don't use AC/DC relays buttons, remove this include 'NodeRelayButtons.h'"
        EventRelayButton ev[0]{};
#    endif
#  endif
        void AutoOff(uint8_t & idx) {
            if (ev[idx].s == LOW)
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
            digitalWrite(ev[idx].pr, ev[idx].s);
            saveState(ev[idx].n, ev[idx].s);
            reportMsg(ev[idx].n, V_STATUS, static_cast<bool>(ev[idx].s));
        }
    
    public:
        NodeRelayButton () {
        }
        NodeRelayButton(NodeLiveLight const *l) {
          light = l;
        }
        ~NodeRelayButton () {
          for (uint8_t i = 0U; i < __NELE(ev); i++) {
            delete ev[i].b;
          }
        }
        void init(uint16_t) {}
        void init() {

          MY_CRITICAL_SECTION {
            for (uint8_t i = 0U; i < __NELE(ev); i++) {

                ev[i].b = new Bounce();

#               if (defined(LIGHT_ON_POWER_BTN) && (LIGHT_ON_POWER_BTN == ON))
                  ev[i].s = HIGH;
#               elif (defined(LIGHT_ON_POWER_BTN) && (LIGHT_ON_POWER_BTN == OFF))
                  ev[i].s = LOW;
#               else
                  ev[i].s = loadState(ev[i].n);
#               endif
                ev[i].e = ((ev[i].s) ? HIGH : LOW);

                pinMode(ev[i].pr, OUTPUT);

                pinMode(ev[i].pb, INPUT_PULLUP);
                digitalWrite(ev[i].pb, HIGH);
                ev[i].b->attach(ev[i].pb);
                ev[i].b->interval(BOUNCER_INTERVAL);

                /*
                PRINTF("-- INIT RELAY BUTTON [%u]: id=%u, pin=%u/%u, state=%u, event=%u\n",
                    (uint16_t) (i + 1U), (uint16_t) ev[i].n,
                    (uint16_t) ev[i].pr, (uint16_t) ev[i].pb,
                    (uint16_t) ev[i].s, (uint16_t) ev[i].e
                );
                */
            }
          }
        }
        bool presentation() {
          
          /*
          PRINTLN("NODE RELAY BUTTON | presentation");
          */
          for (uint8_t i = 0U; i < __NELE(ev); i++) {

            if (!presentSend(ev[i].n, S_BINARY, "Relay.Button.Lights"))
              return false;
            if (!presentSend(ev[i].n, V_STATUS))
              return false;
          }
          return true;
        }
        void data(uint16_t & cnt) {

          for (uint8_t i = 0U; i < __NELE(ev); i++) {

                if (!ev[i].b->update())
                  continue;
                if (!ev[i].b->fell())
                  continue;
                
                ev[i].e = HIGH;
                ev[i].s = !(ev[i].s);
                isChange = true;
          }

          if ((cnt % 70) == 0) {
            for (uint8_t i = 0U; i < __NELE(ev); i++)
              AutoOff(i);
          }

          if (!isChange)
            return;
          
          isChange = false;

          /*
          PRINTLN("-- NODE RELAY BUTTON | begin data");
          */
          for (uint8_t i = 0U; i < __NELE(ev); i++) {

            if (ev[i].e == LOW)
              continue;

            ev[i].e = LOW;
            change(i);
                
            /*
            PRINTF("-- CHANGE RELAY BUTTON | id=%u, pin=%u/%u, state=%u, event=%u\n",
                (uint16_t) ev[i].n, (uint16_t) ev[i].pr, (uint16_t) ev[i].pb,
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
          PRINTF("-- INCOMING RELAY BUTTON | begin MyMessage: %u, index=%u, type: %u\n",
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

#  undef BOUNCER_INTERVAL
# endif
#endif

