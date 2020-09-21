
#if !defined(__MY_SENSOR_RSSI_LEVEL_H)
#define __MY_SENSOR_RSSI_LEVEL_H 1

/* ------- RADIO RSSI LEVEL ------- */

class NodeLiveRssi {
    private:
        bool isChange = false;
        int16_t rssi = 0;
        bool radioQuality() {
#           if defined(MY_GATEWAY_ESP8266)
                int16_t r_ = static_cast<int16_t>(WiFi.RSSI());
#           elif defined(MY_RADIO_RF24)
                int16_t r_ = transportGetSendingRSSI();
#           else
                int16_t r_ = 0;
                return false;
#           endif
            
            r_ = map(r_, -85, -40, 0, 100);
            r_ = ((r_ < 0) ? 0 : ((r_ > 100) ? 100 : r_));
            if (rssi != r_) {
              ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                rssi = r_;
              }
              return true;
            }
            return false;
        }
        uint8_t getId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_RSSI);
        }

    public:
        void init(uint16_t) {}
        void init() {}
        bool presentation() {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                isChange = true;
            }
            return true;
        }
        void data(uint16_t & cnt) {
            if (((cnt % 90) == 0) || (isChange)) {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                    isChange = false;
                }
                if (radioQuality())
                    reportMsg(getId(), I_SIGNAL_REPORT_RESPONSE, static_cast<uint16_t>(rssi));
            }
        }
        bool data(const MyMessage & msg) {
            if ((msg.sensor != getId()) || (msg.type != I_SIGNAL_REPORT_REQUEST))
                return false;
            
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                isChange = true;
            }
            return isChange;
        }
};

#endif
