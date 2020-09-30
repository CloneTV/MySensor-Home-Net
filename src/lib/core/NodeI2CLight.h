#if !defined(__MY_SENSOR_I2C_LIGHT_PROXIMITY_LEVEL_H)
#define __MY_SENSOR_I2C_LIGHT_PROXIMITY_LEVEL_H 1

#  if defined(ENABLE_I2C_SENSOR_ILLUMINATION)
#  if defined(__MY_SENSOR_LIGHT_LEVEL_H)
#    pragma message "WARNING - do not include 'NodeLiveLight.h', sensor ID intersect!"
#  endif

/* ------- LIGHT/PROXIMITY LEVEL AP3216 I2C ------- */

typedef void (*light_input_cb)(float&);

#  define CALIBRATE_LIGHT_MIN 0
#  define CALIBRATE_LIGHT_MAX 50 // 323 lux
#  define CALIBRATE_LENGTH_MIN 0
#  define CALIBRATE_LENGTH_MAX 500
#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#  define POLL_WAIT_SECONDS 30U
#  define IDX_Start     0
#  define IDX_Setup     1

#include <AP3216_WE.h>

class NodeI2CLight : public SensorInterface<NodeI2CLight> {

    public:
        enum LIGHTS {
            NONE = 0,
            DARK,
            GLOOMY,
            DIM,
            LIGHT,
            BRIGHT,
            HILEVEL
        };
        enum LENGTHS {
            NOONE = 0,
            FAR,
            LONG,
            CLOSER,
            OPPOSITE,
            NEARBY
        };

    private:
        bool isAction[2] = {
            true,
            false
        };
        float rawlight = 0;
        uint16_t rawlength = 0;
        NodeI2CLight::LIGHTS  lightState  = NodeI2CLight::LIGHTS::NONE,
                              lightSend   = NodeI2CLight::LIGHTS::NONE;
        NodeI2CLight::LENGTHS lengthState = NodeI2CLight::LENGTHS::NOONE,
                              lengthSend  = NodeI2CLight::LENGTHS::NOONE;
        light_input_cb cb = [](float&){};
                             
        float baselight[6] = {
            2, 4, 8,
            12, 20, 35
        };
        uint16_t baselength[6] = {
            4U, 10U, 25U,
            40U, 55U, 70U
        };
        AP3216_WE *pAP3216 = nullptr;

        uint8_t getLightId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_ILLUMINATION);
        }
        uint8_t getLengthId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_PROXIMITY);
        }
        uint8_t getAutoSetupId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_AUTO_LIGHT_SETUP);
        }
        uint16_t getLightLevel() {
            return static_cast<uint16_t>(
                map(
                    static_cast<uint16_t>(round(rawlight)),
                    CALIBRATE_LIGHT_MIN, CALIBRATE_LIGHT_MAX,
                    0, 100
                )
            );
        }
        uint16_t getLengthLevel() {
            uint16_t x = map(
                            rawlength,
                            CALIBRATE_LENGTH_MIN, CALIBRATE_LENGTH_MAX,
                            0, 100
                        );
            return (100U - x);
        }
        void readSensor() {
            float rawlight_ = pAP3216->getAmbientLight();
            uint16_t rawlength_ = 0U;
            if (!pAP3216->irDataIsOverflowed())
                rawlength_ = max(static_cast<uint16_t>(0U), pAP3216->getProximity());
            pAP3216->setMode(ALS_PS_ONCE);
            readLight(rawlight_);
            if (rawlength_)
                readLength(rawlength_);
            cb(rawlight_);
        }
        void readLight(float & rawlight_) {

            if (rawlight_ == rawlight) {
                lightState = NodeI2CLight::LIGHTS::NONE;
                return;
            }
            rawlight = rawlight_;
                 if (rawlight <= baselight[0]) lightState = NodeI2CLight::LIGHTS::NONE;
            else if (rawlight <= baselight[1]) lightState = NodeI2CLight::LIGHTS::DARK;
            else if (rawlight <= baselight[2]) lightState = NodeI2CLight::LIGHTS::GLOOMY;
            else if (rawlight <= baselight[3]) lightState = NodeI2CLight::LIGHTS::DIM;
            else if (rawlight <= baselight[4]) lightState = NodeI2CLight::LIGHTS::LIGHT;
            else if (rawlight <= baselight[5]) lightState = NodeI2CLight::LIGHTS::BRIGHT;
            else                               lightState = NodeI2CLight::LIGHTS::HILEVEL;
        }
        void readLength(uint16_t & rawlength_) {

            if (rawlength_ == rawlength) {
                lengthState = NodeI2CLight::LENGTHS::NOONE;
                return;
            }
            rawlength = rawlength_;
                 if (rawlength <= baselength[0]) lengthState = NodeI2CLight::LENGTHS::NOONE;
            else if (rawlength <= baselength[1]) lengthState = NodeI2CLight::LENGTHS::FAR;
            else if (rawlength <= baselength[2]) lengthState = NodeI2CLight::LENGTHS::LONG;
            else if (rawlength <= baselength[3]) lengthState = NodeI2CLight::LENGTHS::CLOSER;
            else if (rawlength <= baselength[4]) lengthState = NodeI2CLight::LENGTHS::OPPOSITE;
            else                                 lengthState = NodeI2CLight::LENGTHS::NEARBY;
        }

    public:
        NodeI2CLight() {
            pAP3216 = new AP3216_WE();
        }
        ~NodeI2CLight() {
            delete pAP3216;
        }
        bool go_init() {
            pAP3216->init();
            pAP3216->setLuxRange(RANGE_323);
            pAP3216->setMode(ALS_PS_ONCE);
            pAP3216->setNumberOfLEDPulses(4);
            pAP3216->setPSMeanTime(PS_MEAN_TIME_50);
            return true;
        }
        void setCallBack(light_input_cb c) {
            cb = c;
        }
        int16_t getLightVal() {
            return rawlight;
        }
        int16_t getLengthVal() {
            return rawlength;
        }
        NodeI2CLight::LIGHTS getLightState() {
            return lightState;
        }
        NodeI2CLight::LENGTHS getLengthState() {
            return lengthState;
        }
        bool go_presentation() {
          
            uint8_t lid = getLightId(),
                    pid = getLengthId(),
                    aid = getAutoSetupId();
            if (!presentSend(lid, S_LIGHT_LEVEL))
              return false;
            if (!presentSend(lid, V_LIGHT_LEVEL))
              return false;
            if (!presentSend(pid, S_DISTANCE))
              return false;
            if (!presentSend(pid, V_DISTANCE))
              return false;
            if (!presentSend(aid, S_CUSTOM, "Setup.Auto.Lights"))
              return false;
            if (!presentSend(aid, V_CUSTOM))
              return false;
            return true;
        }
        void go_data(uint16_t & cnt) {

            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isAction[IDX_Start])) {
                readSensor();
                            
                if (((lightState != NodeI2CLight::LIGHTS::NONE) && (lightState != lightSend)) || (isAction[IDX_Start])) {
                    lightSend = lightState;
                    reportMsg(
                        getLightId(),
                        V_LIGHT_LEVEL,
                        rawlight,
                        2
                    );
                }
                if (((lengthState != NodeI2CLight::LENGTHS::NOONE) && (lengthState != lengthSend)) || (isAction[IDX_Start])) {
                    lengthSend = lengthState;
                    reportMsg(
                        getLengthId(),
                        V_DISTANCE,
                        getLengthLevel()
                    );
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
        }
        bool go_data(const MyMessage & msg) {
            
            switch (msg.getType()) {
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
                                const int32_t n = static_cast<int32_t>(ss.toInt());
                                if (n > 0)
                                    baselight[idx++] = static_cast<float>(n);
                            }
                            pos = (i + 1);
                        }
                    }
                    isAction[IDX_Setup] = true;
                    break;
                }
                case V_LIGHT_LEVEL: {
                    if (msg.sensor != getLightId())
                        return false;
                    isAction[IDX_Start] = true;
                    break;
                }
                case V_DISTANCE: {
                    if (msg.sensor != getLengthId())
                        return false;
                    isAction[IDX_Start] = true;
                    break;
                }
                default:
                    return false;
            }
            return true;
        }
};

#  undef POLL_WAIT_SECONDS
#  endif
#endif
