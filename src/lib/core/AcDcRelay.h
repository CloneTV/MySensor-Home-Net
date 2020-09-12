#if !defined(__MY_SENSOR_ACDCRELAY_H)
#define __MY_SENSOR_ACDCRELAY_H 1

#include <Bounce2.h>

struct AcDcRelay {
    private:
        uint8_t state_[size_light_sensors]{};
        Bounce *bouncer_[size_light_sensors]{};
    
    public:
        ~AcDcRelay() {
            for (uint16_t i = 0U; i < size_light_sensors; i++) {
                delete bouncer_[i];
            }
        }
        AcDcRelay() {
            for (uint16_t i = 0U; i < size_light_sensors; i++) {
                bouncer_[i] = new Bounce();
                state_[i] = 0U;
            }
        }
        void init(uint16_t ival = 5U) {

            for (uint16_t i = 0U; i < size_light_sensors; i++) {

                uint16_t sensor = setup_light_sensors[i][INDEX_NODE_SENSOR_ID],
                         pin_b = setup_light_sensors[i][INDEX_PIN_SENSOR_BUTTON],
                         pin_r = setup_light_sensors[i][INDEX_PIN_SENSOR_RELAY];

                pinMode(pin_b, INPUT_PULLUP);
                digitalWrite(pin_b, HIGH);
                bouncer_[i]->attach(pin_b);
                bouncer_[i]->interval(ival);

#               if (defined(LIGHT_ON_POWER) && (LIGHT_ON_POWER >= 0))
                if (((uint16_t)ALL == (uint16_t)LIGHT_ON_POWER) ||
                    (i == (uint16_t)LIGHT_ON_POWER))
                    state_[i] = SENSOR_INIT(ON);
                else
#               endif
                state_[i] = SENSOR_INIT(loadState(sensor));

                pinMode(pin_r, OUTPUT);
                digitalWrite(pin_r, ((state_[i]) ? HIGH : LOW));

#               if defined(ENABLE_DEBUG)
                  yield();
                  PRINTF("-- Set: reley=%u, state=%u\n",
                    sensor, static_cast<uint16_t>(state_[i])
                  );
#               endif
            }
        }
        bool presentation() {
            for (uint16_t i = 0U; i < size_light_sensors; i++) {

                uint8_t sensor = static_cast<uint8_t>(setup_light_sensors[i][INDEX_NODE_SENSOR_ID]);
                
                if (!presentSend(sensor, S_BINARY))
                    return false;
                if (!presentSend(sensor, V_STATUS))
                    return false;
            }
            return true;
        }
        void data() {
            for (uint16_t i = 0U; i < size_light_sensors; i++) {

                if (!bouncer_[i]->update())
                  continue;
                if (!bouncer_[i]->fell())
                  continue;
                  
                uint16_t sensor = setup_light_sensors[i][INDEX_NODE_SENSOR_ID],
                         pin_r = setup_light_sensors[i][INDEX_PIN_SENSOR_RELAY];

                state_[i] = SENSOR_SET(state_[i]);
                MyMessage msg(sensor, V_STATUS);
                send(
                  msg.set(static_cast<bool>(state_[i])),
                  true
                );
                digitalWrite(pin_r, state_[i]);
                
#               if defined(ENABLE_DEBUG)
                  yield();
                  PRINTF("-- Change (button): sensor=%u, state=%u\n",
                      sensor, static_cast<uint16_t>(state_[i])
                  );
#               endif

                yield();
                INFO_LED(2);
            }
        }
        bool data(const MyMessage & msg) {
          PRINTF("-- RELAY msg.sensor: %u, type: %u\n", msg.sensor, msg.type);
            if (msg.type == V_STATUS) {
              if ((msg.sensor >= size_light_sensors) || (state_[msg.sensor] == msg.getByte()))
                return false;

              state_[msg.sensor] = SENSOR_SET(state_[msg.sensor]);
              uint16_t pin_r = setup_light_sensors[msg.sensor][INDEX_PIN_SENSOR_RELAY];
              digitalWrite(pin_r, state_[msg.sensor]);
              saveState(pin_r, state_[msg.sensor]);
              INFO_LED(3);

#             if defined(ENABLE_DEBUG)
                yield();
                PRINTF("-- Change (remote): sensor=%u, state=%u\n",
                  msg.sensor, static_cast<uint16_t>(state_[msg.sensor])
                );
#             endif
              return true;
            }
            return false;
        }
};

#endif

