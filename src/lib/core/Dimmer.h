#if !defined(__MY_SENSOR_DIMMER_H)
#define __MY_SENSOR_DIMMER_H 1

struct Dimmer {
#   define FADE_DELAY 10

    private:
        int16_t cval = 0;
        void fade(int16_t pin, int16_t val) {
            int16_t d = (((val - cval) < 0) ? -1 : 1);
            while (cval != val) {
                cval += d;
                analogWrite(pin, (int)((cval / 100.) * 255));
                wait(FADE_DELAY);
            }
        }
        void lightMsg(uint8_t id, bool b) {
            MyMessage mmsg(id, V_LIGHT);
            send(mmsg.set(b), true);
        }
        void dimmMsg(uint8_t id) {
            MyMessage mmsg(id, V_DIMMER);
            send(mmsg.set(cval), true);
        }
   
    public:
        ~Dimmer() {
        }
        Dimmer() {
        }
        void init() {
            
            for (uint16_t i = 0U; i < size_dimmer_sensors; i++) {

                uint16_t sensor = setup_dimmer_sensors[i][INDEX_NODE_SENSOR_ID],
                         pin_d = setup_dimmer_sensors[i][INDEX_PIN_SENSOR_DIMMER];

                pinMode(pin_d, OUTPUT);
                setupPwm(pin_d);
            }
        }
        bool presentation() {
            for (uint16_t i = 0U; i < size_dimmer_sensors; i++) {

                uint8_t sensor = static_cast<uint8_t>(setup_dimmer_sensors[i][INDEX_NODE_SENSOR_ID]);
                
                if (!presentSend(sensor, S_DIMMER))
                    return false;
                if (!presentSend(sensor, V_DIMMER))
                    return false;
            }
            return true;
        }
        void data() {
        }
        bool data(const MyMessage & msg) {

            switch (msg.getType()) {
                case V_LIGHT: {
                  bool old = (cval > 0);
                  fade(msg.sensor, (old) ? 0 : 100);
                  lightMsg(msg.sensor, old);
                  break;
                }
                case V_DIMMER: {
                  bool    old = (cval > 0);
                  int16_t val = (atoi(msg.data) * 100);
                  val = ((val > 100) ? 100 : ((val < 0) ? 0 : val));
                  
                  fade(msg.sensor, val);
                  dimmMsg(msg.sensor);
                  if (!old && (cval > 0))
                    lightMsg(msg.sensor, true);
                  else if (old && !cval)
                    lightMsg(msg.sensor, false);
                  break;
                }
                default:
                    return false;
            }
            return true;
        }
};

#endif
