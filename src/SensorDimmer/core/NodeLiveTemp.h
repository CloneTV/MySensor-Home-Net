
#if !defined(__MY_SENSOR_TEMP_LEVEL_H)
#define __MY_SENSOR_TEMP_LEVEL_H 1

/* ------- BUILD-IN TEMP SENSOR ------- */

#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#define POLL_WAIT_SECONDS 183U

class NodeLiveTemp {
    private:
        bool isStart = true;
        int8_t temp = 0;
        bool chipTemperature() {
            int8_t t_ = hwCPUTemperature();
            if (t_ == -127)
              return false;
            return ((temp != t_) ? __extension__ ({temp = t_; true;}) : false);
        }
        uint8_t getId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_TEMP);
        }

    public:
        void init(uint16_t) {}
        void init() {}
        bool presentation() {
            /*
              PRINTLN("NODE TEMP | presentation");
            */
            if (!presentSend(getId(), S_TEMP, "Int.Temp"))
              return false;
            
            return true;
        }
        void data(uint16_t & cnt) {

            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isStart)) {
                if (chipTemperature())
                    reportMsg(getId(), V_TEMP, temp);
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
