
#if !defined(__MY_SENSOR_RSSI_LEVEL_H)
#define __MY_SENSOR_RSSI_LEVEL_H 1

/* ------- RADIO RSSI LEVEL ------- */

#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#define POLL_WAIT_SECONDS 182U

class NodeLiveRssi {
    private:
        bool isStart = true;
        int16_t rssi = 0;
        bool radioQuality() {
            int16_t r_;
#           if defined(MY_GATEWAY_ESP8266)
                r_ = static_cast<int16_t>(WiFi.RSSI());
#           elif defined(MY_RADIO_RF24)
                r_ = transportGetSendingRSSI();
#           else
                return false;
#           endif
            
            r_ = map(r_, -85, -40, 0, 100);
            r_ = ((r_ < 0) ? 0 : ((r_ > 100) ? 100 : r_));
            if (rssi != r_) {
              rssi = r_;
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

            uint8_t id = getId();
            if (!presentSend(id, S_SOUND, "RSSI"))
              return false;
            if (!presentSend(id, V_LEVEL))
              return false;
            return true;
        }
        void data(uint16_t & cnt) {
            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isStart)) {
                if (radioQuality())
                    reportMsg(getId(), V_LEVEL, static_cast<uint16_t>(rssi));
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
