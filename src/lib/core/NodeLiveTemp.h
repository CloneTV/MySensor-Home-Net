
#if !defined(__MY_SENSOR_TEMP_LEVEL_H)
#define __MY_SENSOR_TEMP_LEVEL_H 1

/* ------- BUILD-IN TEMP SENSOR ------- */

#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#define POLL_WAIT_SECONDS 183U

class NodeLiveTemp : public SensorInterface<NodeLiveTemp> {
    private:
        bool isAction = true;
        int8_t temp = 0;
        bool chipTemperature() {
            int8_t t_ = hwCPUTemperature();
            if (t_ == -127)
              return false;
            return ((temp != t_) ? __extension__ ({temp = t_; true;}) : false);
        }
        uint8_t getTempId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_TEMP);
        }

    public:
        void enable() {
            isAction = true;
        }
        bool go_init() {
            return true;
        }
        bool go_presentation() {
            /*
              PRINTLN("NODE TEMP | presentation");
            */
            if (!presentSend(getTempId(), S_TEMP, "Int.Temp"))
              return false;
            
            return true;
        }
        void go_data(uint16_t & cnt) {

            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isAction)) {
                if (chipTemperature())
                    reportMsg(getTempId(), V_TEMP, temp, false);
                if (isAction)
                    isAction = false;
            }
        }
        bool go_data(const MyMessage & msg) {
            switch (msg.getType()) {
                case V_TEMP: {
                    if (msg.sensor != getTempId())
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
