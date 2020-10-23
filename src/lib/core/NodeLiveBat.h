
#if !defined(__MY_SENSOR_BAT_LEVEL_H)
#define __MY_SENSOR_BAT_LEVEL_H 1

/* ------- BUILD-IN BATTERY CHARGE LEVEL ------- */

#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#define POLL_WAIT_SECONDS 300U

class NodeLiveBat : public SensorInterface<NodeLiveBat> {
    private:
        bool isAction = true;
        uint8_t proc = 0U;
        float   volt = 0.00f;
        bool chipVoltage() {
#           if defined(MY_GATEWAY_ESP8266)
                uint16_t v_ = ESP.getVcc();
                volt = ((float)v_/1024.0f);
                proc = static_cast<uint8_t>(map(v_, 0, 1024, 0, 100));
                return true;
#           else
                int16_t v_ = analogRead(INTERNAL_LIVE_VOLT_PIN);
                volt = v_ * 0.003363075;
                v_ /= 10;
                v_ = ((v_ < 0) ? 0 : ((v_ > 100) ? 100 : v_));
                
                if (proc != v_) {
                    proc = static_cast<uint8_t>(v_);
                    return true;
                }
                return false;
#           endif
        }
        uint8_t getVoltId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_VOLT);
        }

    public:
        bool go_init() {
#           if defined(MY_GATEWAY_ESP8266)
                ADC_MODE(ADC_VCC);
#           elif defined(__AVR_ATmega2560__)
                analogReference(INTERNAL1V1);
#           else
                analogReference(INTERNAL);
#           endif
            return true;
        }
        bool go_presentation() {
            if (!presentSend(getVoltId(), S_MULTIMETER, "Int.Bat.Volt"))
                return false;
            if (!presentSend(getVoltId(), V_VOLTAGE))
                return false;
            return true;
        }
        void go_data(uint16_t & cnt) {
            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isAction)) {
                if (chipVoltage())
                    sendBatteryLevel(proc, false);
                if (!isnan(volt))
                    reportMsg(getVoltId(), V_VOLTAGE, volt, 2U, false);

                if (isAction)
                    isAction = false;
            }
        }
        bool go_data(const MyMessage & msg) {
            switch (msg.getType()) {
                case V_VOLTAGE: {
                    if (msg.sensor != getVoltId())
                        return false;
                    break;
                }
                default:
                    return false;
            }
            isAction = true;
            return isAction;
        }
};

#  undef POLL_WAIT_SECONDS
#endif
