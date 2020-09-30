#if !defined(__MY_SENSOR_LIGHT_LEVEL_H)
#define __MY_SENSOR_LIGHT_LEVEL_H 1


#  if defined(ENABLE_LIVE_SENSOR_ILLUMINATION)
#  if defined(__MY_SENSOR_I2C_LIGHT_PROXIMITY_LEVEL_H)
#    pragma message "WARNING - do not include 'NodeI2CLight.h', sensor ID intersect!"
#  endif

/* ------- LIGHT LEVEL SENSOR ------- */

typedef void (*light_input_cb)(int16_t&);

#  define CALIBRATE_MIN 0
#  define CALIBRATE_MAX 900
#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#define POLL_WAIT_SECONDS 90U

#define IDX_Calculate 0
#define IDX_Change    1
#define IDX_Start     2
#define IDX_Setup     3

class NodeLiveLight : public SensorInterface<NodeLiveLight> {

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
        bool isAction[4] = {
            true,   // is Calculate (IDX_Calculate)
            true,   // is Change    (IDX_Change)
            true,   // is Start     (IDX_Start)
            false   // is Setup     (IDX_Setup)
        };
        int16_t rawlight = 0,
                offset = 0;
        NodeLiveLight::LIGHTS state = NodeLiveLight::LIGHTS::None,
                              stsend = NodeLiveLight::LIGHTS::None;
        light_input_cb cb = [](int16_t&){};
        /*
            // default base value
        uint16_t baselight[6] = {
            5U, 50U, 150U,
            300U, 500U, 700U
        };
        */
        uint16_t baselight[6] = {
            2U, 5U, 10U,
            15U, 25U, 35U
        };
        void calibrate() {
            int16_t v1 = 0, v2 = 0;
            (void) analogRead(INTERNAL_LIVE_ILLUMINATION_PIN);
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
        uint8_t getAutoSetupId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_AUTO_LIGHT_SETUP);
        }
        uint8_t getLummId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_ILLUMINATION);
        }
        uint16_t getLevel() {
            return static_cast<uint16_t>(map(rawlight, CALIBRATE_MIN, CALIBRATE_MAX, 0, 100));
        }

    public:
        void setCallBack(light_input_cb c) {
            cb = c;
        }
        bool go_init() {
            calibrate();
            isAction[IDX_Calculate] = static_cast<bool>(loadState(getAutoId()));
            return true;
        }
        NodeLiveLight::LIGHTS read() {

            int16_t rawlight_ = analogRead(INTERNAL_LIVE_ILLUMINATION_PIN);
            rawlight_ = ((rawlight_ <= offset) ? rawlight_ : (rawlight_ - offset));
            if ((rawlight_ == rawlight) || (rawlight_ < 0))
                return NodeLiveLight::LIGHTS::None;
                
            rawlight = rawlight_;
                 if (rawlight <= baselight[0]) state = NodeLiveLight::LIGHTS::None;
            else if (rawlight <= baselight[1]) state = NodeLiveLight::LIGHTS::Dark;
            else if (rawlight <= baselight[2]) state = NodeLiveLight::LIGHTS::Gloomy;
            else if (rawlight <= baselight[3]) state = NodeLiveLight::LIGHTS::Dim;
            else if (rawlight <= baselight[4]) state = NodeLiveLight::LIGHTS::Light;
            else if (rawlight <= baselight[5]) state = NodeLiveLight::LIGHTS::Bright;
            else                               state = NodeLiveLight::LIGHTS::High;

            /*
            PRINTF("-- LIGHT LEVEL: %d/%u/%d (%u|%u)\n", rawlight, getLevel(), offset, (uint16_t)state, (uint16_t)stsend);
             */
        }
        int16_t getVal() {
            return rawlight;
        }
        NodeLiveLight::LIGHTS getState() {
            return ((isAction[IDX_Calculate]) ? state : NodeLiveLight::LIGHTS::None);
        }
        bool go_presentation() {
          
            /*
            PRINTLN("NODE LIGHTS | presentation");
            */
            uint8_t aid = getAutoId(),
                    sid = getAutoSetupId(),
                    lid = getLummId();
            if (!presentSend(lid, S_LIGHT_LEVEL))
              return false;
            if (!presentSend(lid, V_LIGHT_LEVEL))
              return false;
            if (!presentSend(aid, S_BINARY, "Auto.Lights"))
              return false;
            if (!presentSend(aid, V_STATUS))
              return false;
            if (!presentSend(sid, S_CUSTOM, "Setup.Auto.Lights"))
              return false;
            if (!presentSend(sid, V_CUSTOM))
              return false;
            return true;
        }
        void go_data(uint16_t & cnt) {

            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isAction[IDX_Start])) {
                (void) read();
                if ((stsend != state) || (isAction[IDX_Start])) {
                    /*
                    PRINTF("-- SEND LEVEL: %d/%u (%u|%u)\n", rawlight, getLevel(), (uint16_t)state, (uint16_t)stsend);
                    */
                    stsend = state;
                    int16_t level = getLevel();
                    reportMsg(
                        getLummId(),
                        V_LIGHT_LEVEL,
                        level
                    );
                    cb(level);
                }
                if (isAction[IDX_Start])
                    isAction[IDX_Start] = false;
            }
            if (isAction[IDX_Setup]) {
                isAction[IDX_Setup] = false;
                
                String sdata = String("{\"data\":[");
                for (uint16_t i = 0U; i < __NELE(baselight); i++) {
                    sdata.concat(baselight[i]);
                    if (i < (__NELE(baselight) - 1U)) {
                        sdata.concat(",");
                    }
                }
                sdata.concat("]}");
                reportMsg(getAutoSetupId(), V_CUSTOM, sdata.c_str());
            }
            if (isAction[IDX_Change]) {
                isAction[IDX_Change] = false;
                reportMsg(getAutoId(), V_STATUS, static_cast<bool>(isAction[IDX_Calculate]));
            }
        }
        bool go_data(const MyMessage & msg) {
            
            switch (msg.getType()) {
                case V_LIGHT_LEVEL: {
                    if (msg.sensor != getLummId())
                        return false;
                    isAction[IDX_Start] = true;
                    break;
                }
                case V_STATUS: {
                    if (msg.sensor != getAutoId())
                        return false;
                    if (isAction[IDX_Calculate] == msg.getBool())
                        break;
                    
                    isAction[IDX_Calculate] = msg.getBool();
                    isAction[IDX_Change] = true;
                    saveState(msg.sensor, static_cast<uint8_t>(isAction[IDX_Calculate]));
                    break;
                }
                case V_CUSTOM: {
                    if (msg.sensor != getAutoSetupId())
                        return false;

                    String s = String(msg.getString());
                    for (
                        uint16_t i = 0U, pos = 0U, idx = 0U;
                        ((i < s.length()) && (idx < __NELE(baselight)));
                        i++) {
                        const char c = s.charAt(i);
                        if ((c == ';') || (c == ',') || (c == '|')) {
                            String ss = s.substring(pos, i);
                            if (ss.length() > 0) {
                                const int16_t n = static_cast<int16_t>(ss.toInt());
                                if (n > 0)
                                    baselight[idx++] = static_cast<uint16_t>(n);
                            }
                            pos = (i + 1);
                        }
                    }
                    isAction[IDX_Setup] = true;
                    break;
                }
                default:
                    return false;
            }
            return true;
        }
};

#  undef POLL_WAIT_SECONDS
#  undef IDX_Calculate
#  undef IDX_Change
#  undef IDX_Start
#  undef IDX_Setup
#  endif
#endif
