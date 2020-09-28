
#if !defined(__MY_SENSOR_RSSI_LEVEL_H)
#define __MY_SENSOR_RSSI_LEVEL_H 1

/* ------- RADIO RSSI LEVEL ------- */

#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#define POLL_WAIT_SECONDS 182U
#define MYCONTROLLER_ENGINE 1 /* https://www.mycontroller.org/ */

class NodeLiveRssi : public SensorInterface<NodeLiveRssi> {
    private:
        bool isAction = true;
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
        bool go_init() {
            return true;
        }
        bool go_presentation() {

#           if !defined(MYCONTROLLER_ENGINE)
            uint8_t id = getId();
            if (!presentSend(id, S_SOUND, "RSSI"))
              return false;
            if (!presentSend(id, V_LEVEL))
              return false;
#           endif
            return true;
        }
        void go_data(uint16_t & cnt) {
            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isAction)) {
                if (radioQuality()) {
#                   if !defined(MYCONTROLLER_ENGINE)
                    reportMsg(getId(), V_LEVEL, static_cast<uint16_t>(rssi));
#                   else
                    /*
                        // Compare memory size, snprintf -> String
                    char *buff = new char[18]{};
                    (void) snprintf(buff, 17, "rssi:%d", rssi);
                    reportMsg(getId(), V_VAR5, buff);
                    delete [] buff;
                    */
                    String s = String("rssi:");
                    s.concat(rssi);
                    reportMsg(getId(), V_VAR5, s.c_str());
#                   endif
                }
                if (isAction)
                    isAction = false;
            }
        }
        bool go_data(__attribute__ (( __unused__ )) const MyMessage&) {
            return false;
        }
};

#  undef POLL_WAIT_SECONDS
#endif
