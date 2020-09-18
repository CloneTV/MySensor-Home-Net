#if !defined(__MY_SENSOR_LIGHT_LEVEL_H)
#define __MY_SENSOR_LIGHT_LEVEL_H 1

/* ------- LIGHT LEVEL SENSOR ------- */

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
            while (millis() < 2500) {
                int16_t v = analogRead(INTERNAL_LIVE_ILLUMINATION_PIN);
                v1 = ((v1 < v) ? v : v1);
                v2 = ((v2 > v) ? v : v2);
                yield();
            }
            offset = (v1 - v2);
        }
        uint8_t getAutoId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_AUTO_LIGHT);
        }
        uint8_t getLummId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_ILLUMINATION);
        }

    public:
        void init() {
            calibrate();
            isCalculate = static_cast<bool>(loadState(getAutoId()));
        }
        LIGHTS read() {

            int16_t val_ = analogRead(INTERNAL_LIVE_ILLUMINATION_PIN);
            val_ = ((val_ <= offset) ? val_ : (val_ - offset));
            if (val_ == val)
                return LIGHTS::None;
            val = val_;

                 if (val <= 10)  state = LIGHTS::None;
            else if (val <= 50)  state = LIGHTS::Dark;
            else if (val <= 200) state = LIGHTS::Gloomy;
            else if (val <= 500) state = LIGHTS::Dim;
            else if (val <= 700) state = LIGHTS::Light;
            else if (val <= 800) state = LIGHTS::Bright;
            else                 state = LIGHTS::High;

            PRINTF("-- LIGHT LEVEL SENSOR: %d/%d (%u)\n", val, offset, (uint16_t)state);
        }
        int16_t getVal() {
            return val;
        }
        LIGHTS getState() {
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
            if (!presentSend(aid, S_BINARY))
              return false;
            if (!presentSend(aid, V_STATUS))
              return false;
            return true;
        }
        void data(uint16_t & cnt) {

            if ((cnt % 60) == 0) {
                if (read() != LIGHTS::None)
                    reportMsg(getLummId(), V_LIGHT_LEVEL, static_cast<uint16_t>(val));
            }

            if (!isChange)
                return;
            
            isChange = false;
            reportMsg(getAutoId(), V_STATUS, isCalculate);
        }
        bool data(const MyMessage & msg) {
            
            if ((msg.sensor != INTERNAL_LIVE_AUTO_LIGHT) || (msg.type != V_STATUS))
                return false;

            if (isCalculate == msg.getBool())
                return true;

            isCalculate = msg.getBool();
            isChange = true;
            saveState(msg.sensor, isCalculate);
            return isChange;
        }
};

#endif
