#if !defined(__MY_SENSOR_LIGHT_LEVEL_H)
#define __MY_SENSOR_LIGHT_LEVEL_H 1

/* ------- LIGHT LEVEL SENSOR ------- */

const char PROGMEM btn_name[] = "Auto Lights";

class NodeLiveLight {

    public:
        enum LIGHTS {
            None = 0,
            Dark,
            Gloomy,
            Dim,
            Light,
            Bright,
            High
        };

    private:
        bool isCalculate = true,
             isChange = false;
        int16_t val = 0U,
                offset = 0U;
        LIGHTS state = LIGHTS::None;
        void calibrate() {
            int16_t v1 = 0, v2 = 0;
            while (millis() < 5000) {
                wait(100);
                int16_t v = analogRead(INTERNAL_LIVE_ILLUMINATION_PIN);
                v1 = ((v1 < v) ? v : v1);
                v2 = ((v2 > v) ? v : v2);
                yield();
            }
            offset = (v1 - v2);
            if (offset > 30)
                offset = 0;
        }
        uint8_t getAutoId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_AUTO_LIGHT);
        }
        uint8_t getLummId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_ILLUMINATION);
        }

    public:
        void init(uint16_t) {}
        void init() {
            calibrate();
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                isCalculate = static_cast<bool>(loadState(getAutoId()));
            }
        }
        NodeLiveLight::LIGHTS read() {

            int16_t val_ = analogRead(INTERNAL_LIVE_ILLUMINATION_PIN);
            val_ = ((val_ <= offset) ? val_ : (val_ - offset));
            if (val_ == val)
                return LIGHTS::None;

            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                val = val_;
            }
                 if (val <= 10)  state = LIGHTS::None;
            else if (val <= 50)  state = LIGHTS::Dark;
            else if (val <= 150) state = LIGHTS::Gloomy;
            else if (val <= 300) state = LIGHTS::Dim;
            else if (val <= 500) state = LIGHTS::Light;
            else if (val <= 700) state = LIGHTS::Bright;
            else                 state = LIGHTS::High;

            PRINTF("-- LIGHT LEVEL SENSOR: %d/%d (%u)\n", val, offset, (uint16_t)state);
        }
        int16_t getVal() {
            return val;
        }
        NodeLiveLight::LIGHTS getState() {
            return ((isCalculate) ? state : LIGHTS::None);
        }
        bool presentation() {
          
            /*
            PRINTLN("NODE LIGHTS | presentation");
            */
            uint8_t aid = getAutoId(),
                    lid = getLummId();
            if (!presentSend(lid, S_LIGHT_LEVEL))
              return false;
            if (!presentSend(lid, V_LIGHT_LEVEL))
              return false;
            if (!presentSend(aid, S_BINARY, btn_name))
              return false;
            if (!presentSend(aid, V_STATUS))
              return false;
            
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                isChange = true;
            }
            return true;
        }
        void data(uint16_t & cnt) {

            if ((cnt % 60) == 0) {
                if (read() != LIGHTS::None)
                    reportMsg(getLummId(), V_LIGHT_LEVEL, static_cast<uint16_t>(val));
            }

            if (!isChange)
                return;
            
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                isChange = false;
            }
            reportMsg(getAutoId(), V_STATUS, isCalculate);
        }
        bool data(const MyMessage & msg) {
            
            if ((msg.sensor != getAutoId()) || (msg.type != V_STATUS))
                return false;

            if (isCalculate == msg.getBool())
                return true;

            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                isCalculate = msg.getBool();
                isChange = true;
            }
            saveState(msg.sensor, isCalculate);
            return isChange;
        }
};

#endif
