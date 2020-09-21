
#if !defined(__MY_SENSOR_BAT_LEVEL_H)
#define __MY_SENSOR_BAT_LEVEL_H 1

/* ------- BUILD-IN BATTERY CHARGE LEVEL ------- */

class NodeLiveBat {
    private:
        bool isChange = false;
        uint8_t volt = 0U;
        bool chipVoltage() {
#           if defined(MY_GATEWAY_ESP8266)
                int16_t v_ = ESP.getVcc();
#           else
                int16_t v_ = analogRead(INTERNAL_LIVE_VOLT_PIN) / 10;
#           endif

            v_ = ((v_ < 0) ? 0 : ((v_ > 100) ? 100 : v_));
            if (volt != v_) {
              ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                volt = static_cast<uint8_t>(v_);
              }
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
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                isChange = true;
            }
            return isChange;
        }
        void data(uint16_t & cnt) {
            if (((cnt % 90) == 0) || (isChange)) {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                    isChange = false;
                }
                if (chipVoltage())
                    sendBatteryLevel(volt, false);
            }
        }
        bool data(const MyMessage & msg) {
            if ((msg.sensor != getId()) ||
                ((msg.type != I_BATTERY_LEVEL) &&
                 (msg.type != V_VOLTAGE)))
                return false;

            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                isChange = true;
            }
            return isChange;
        }
};

#endif
