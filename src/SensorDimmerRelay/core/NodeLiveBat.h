
#if !defined(__MY_SENSOR_BAT_LEVEL_H)
#define __MY_SENSOR_BAT_LEVEL_H 1

/* ------- BUILD-IN BATTERY CHARGE LEVEL ------- */

#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#define POLL_WAIT_SECONDS 181U

class NodeLiveBat {
    private:
        bool isStart = true;
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
        void init(uint16_t) {}
        void init() {
#           if defined(MY_GATEWAY_ESP8266)
                ADC_MODE(ADC_VCC);
#           elif defined(__AVR_ATmega2560__)
                analogReference(INTERNAL1V1);
#           else
                analogReference(INTERNAL);
#           endif
        }
        bool presentation() {
            return true;
        }
        void data(uint16_t & cnt) {
            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isStart)) {
                if (chipVoltage())
                    sendBatteryLevel(volt, false);
                if (isStart)
                    isStart = false;
            }
        }
        bool data(const MyMessage & msg) {
            return false;
        }
};

#  undef POLL_WAIT_SECONDS
#endif
