#if !defined(__MY_SENSOR_I2C_LIGHT_PROXIMITY_LEVEL_H)
#define __MY_SENSOR_I2C_LIGHT_PROXIMITY_LEVEL_H 1

#  if defined(__MY_SENSOR_LIGHT_LEVEL_H)
#    pragma message "WARNING - do not include 'NodeLiveLight.h', sensor ID intersect!"
#  endif

/* ------- LIGHT/PROXIMITY LEVEL AP3216 I2C ------- */

#  define CALIBRATE_LIGHT_MIN 0
#  define CALIBRATE_LIGHT_MAX 50
#  define CALIBRATE_LENGTH_MIN 0
#  define CALIBRATE_LENGTH_MAX 500
#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#define POLL_WAIT_SECONDS 3U

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
        bool isAction = true;
        float rawlight = 0;
        uint16_t rawlength = 0;
        NodeI2CLight::LIGHTS  lightState  = NodeI2CLight::LIGHTS::NONE,
                              lightSend   = NodeI2CLight::LIGHTS::NONE;
        NodeI2CLight::LENGTHS lengthState = NodeI2CLight::LENGTHS::NOONE,
                              lengthSend  = NodeI2CLight::LENGTHS::NOONE;
                             
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
            /*
            uint16_t x = map(
                            rawlength,
                            CALIBRATE_LENGTH_MIN, CALIBRATE_LENGTH_MAX,
                            0, 100
                        );
                        */
            uint16_t x = rawlength;
            if (x == 65436)
                x = 0U;
            else if (x > 200)
                x = 300U - x;
            else if (x > 100)
                x = 200U - x;
            else if (x < 100)
                x = 100U - x;

            PRINTF("-- getLengthLevel(): %u/%u = %d\n", rawlength, x, lengthState);
            return ((x >= 98U) ? 0U : x);
        }
        void readSensor() {
            float rawlight_ = pAP3216->getAmbientLight();
            uint16_t rawlength_ = max(static_cast<uint16_t>(0U), pAP3216->getProximity());
            pAP3216->setMode(ALS_PS_ONCE);
            readLight(rawlight_);
            readLength(rawlength_);
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
            return true;
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
                    pid = getLengthId();
            if (!presentSend(lid, S_LIGHT_LEVEL))
              return false;
            if (!presentSend(lid, V_LIGHT_LEVEL))
              return false;
            if (!presentSend(pid, S_DISTANCE))
              return false;
            if (!presentSend(pid, V_DISTANCE))
              return false;
            return true;
        }
        void go_data(uint16_t & cnt) {

            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isAction)) {
                readSensor();
                            
                if ((lightState != NodeI2CLight::LIGHTS::NONE) && (lightState != lightSend)) {
                    lightSend = lightState;
                    reportMsg(
                        getLightId(),
                        V_LIGHT_LEVEL,
                        rawlight,
                        2
                    );
                }
                if ((lengthState != NodeI2CLight::LENGTHS::NOONE) && (lengthState != lengthSend)) {
                    lengthSend = lengthState;
                    reportMsg(
                        getLengthId(),
                        V_DISTANCE,
                        getLengthLevel()
                    );
                }
                if (isAction)
                    isAction = false;
            }
        }
        bool go_data(const MyMessage & msg) {
            
            switch (msg.getType()) {
                case V_LIGHT_LEVEL: {
                    if (msg.sensor != getLightId())
                        return false;
                    break;
                }
                case V_DISTANCE: {
                    if (msg.sensor != getLengthId())
                        return false;
                    break;
                }
                default:
                    return false;
            }
            isAction = true;
            return true;
        }
};

#  undef POLL_WAIT_SECONDS
#endif
