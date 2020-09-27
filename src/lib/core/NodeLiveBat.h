
#if !defined(__MY_SENSOR_BAT_LEVEL_H)
#define __MY_SENSOR_BAT_LEVEL_H 1

/* ------- BUILD-IN BATTERY CHARGE LEVEL ------- */

#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#define POLL_WAIT_SECONDS 181U

class NodeLiveBat : public SensorInterface<NodeLiveBat> {
    private:
        bool isAction = true;
        uint8_t volt = 0U;
        bool chipVoltage() {
#           if defined(MY_GATEWAY_ESP8266)
                int16_t v_ = ESP.getVcc();
#           else
                int16_t v_ = analogRead(INTERNAL_LIVE_VOLT_PIN) / 10;
#           endif

            v_ = ((v_ < 0) ? 0 : ((v_ > 100) ? 100 : v_));
            if (volt != v_) {
              volt = static_cast<uint8_t>(v_);
              return true;
            }
            return false;
        }
        uint8_t getId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_VOLT);
        }

    public:
        void go_init() {
#           if defined(MY_GATEWAY_ESP8266)
                ADC_MODE(ADC_VCC);
#           elif defined(__AVR_ATmega2560__)
                analogReference(INTERNAL1V1);
#           else
                analogReference(INTERNAL);
#           endif
        }
        bool go_presentation() {
            return true;
        }
        void go_data(uint16_t & cnt) {
            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isAction)) {
                if (chipVoltage())
                    sendBatteryLevel(volt, false);
                if (isAction)
                    isAction = false;
            }
        }
        bool go_data(__attribute__ (( __unused__ )) const MyMessage & msg) {
            return false;
        }
};

#  undef POLL_WAIT_SECONDS
#endif
