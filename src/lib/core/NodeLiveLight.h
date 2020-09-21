#if !defined(__MY_SENSOR_LIGHT_LEVEL_H)
#define __MY_SENSOR_LIGHT_LEVEL_H 1

/* ------- LIGHT LEVEL SENSOR ------- */

#  define CALIBRATE_MIN 0
#  define CALIBRATE_MAX 900
#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#define POLL_WAIT_SECONDS 60U

// const char PROGMEM btn_name[] = "Auto Lights";

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
             isChange = false,
             isStart = true;
        int16_t rawlight = 0,
                offset = 0;
        NodeLiveLight::LIGHTS state = NodeLiveLight::LIGHTS::None,
                              stsend = NodeLiveLight::LIGHTS::None;
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
        uint16_t getLevel() {
            return static_cast<uint16_t>(map(rawlight, CALIBRATE_MIN, CALIBRATE_MAX, 0, 100));
        }

    public:
        void init(uint16_t) {}
        void init() {
            calibrate();
            isCalculate = static_cast<bool>(loadState(getAutoId()));
        }
        NodeLiveLight::LIGHTS read() {

            int16_t rawlight_ = analogRead(INTERNAL_LIVE_ILLUMINATION_PIN);
            rawlight_ = ((rawlight_ <= offset) ? rawlight_ : (rawlight_ - offset));
            if ((rawlight_ == rawlight) || (rawlight_ < 0))
                return NodeLiveLight::LIGHTS::None;
                
            rawlight = rawlight_;
                 if (rawlight <= 5)   state = NodeLiveLight::LIGHTS::None;
            else if (rawlight <= 50)  state = NodeLiveLight::LIGHTS::Dark;
            else if (rawlight <= 150) state = NodeLiveLight::LIGHTS::Gloomy;
            else if (rawlight <= 300) state = NodeLiveLight::LIGHTS::Dim;
            else if (rawlight <= 500) state = NodeLiveLight::LIGHTS::Light;
            else if (rawlight <= 700) state = NodeLiveLight::LIGHTS::Bright;
            else                      state = NodeLiveLight::LIGHTS::High;

            /*
            PRINTF("-- LIGHT LEVEL: %d/%u/%d (%u|%u)\n", rawlight, getLevel(), offset, (uint16_t)state, (uint16_t)stsend);
             */
        }
        int16_t getVal() {
            return rawlight;
        }
        NodeLiveLight::LIGHTS getState() {
            return ((isCalculate) ? state : NodeLiveLight::LIGHTS::None);
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
            if (!presentSend(aid, S_BINARY, "Auto.Lights"))
              return false;
            if (!presentSend(aid, V_STATUS))
              return false;
            return true;
        }
        void data(uint16_t & cnt) {

            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isStart)) {
                (void) read();
                if (stsend != state) {
                    /*
                    PRINTF("-- SEND LEVEL: %d/%u (%u|%u)\n", rawlight, getLevel(), (uint16_t)state, (uint16_t)stsend);
                    */
                    stsend = state;
                    reportMsg(
                        getLummId(),
                        V_LIGHT_LEVEL,
                        getLevel()
                    );
                }
                if (isStart)
                    isStart = false;
            }

            if (!isChange)
                return;
            
            isChange = false;
            reportMsg(getAutoId(), V_STATUS, isCalculate);
        }
        bool data(const MyMessage & msg) {
            
            if ((msg.sensor != getAutoId()) || (msg.type != V_STATUS))
                return false;

            if (isCalculate == msg.getBool())
                return true;

            isCalculate = msg.getBool();
            isChange = true;
            saveState(msg.sensor, isCalculate);
            return isChange;
        }
};

#  undef POLL_WAIT_SECONDS
#endif
