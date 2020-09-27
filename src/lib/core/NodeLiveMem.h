#if !defined(__MY_SENSOR_MEMORY_FREE_H)
#define __MY_SENSOR_MEMORY_FREE_H 1

/* ------- MEMORY FREE ------- */
#  if !defined(__AVR_INTERNAL_LIVE_COMPATIBLE__)
#    ifdef __arm__
        extern "C" char* sbrk(int incr);
#    else
        extern char *__brkval;
#    endif
#  endif

class NodeLiveMem : public SensorInterface<NodeLiveMem> {
    private:
        bool isAction = true;
        uint16_t getFreeMem() {
#           if defined(__AVR_INTERNAL_LIVE_COMPATIBLE__)
                uint8_t *h, *s;
                s = (uint8_t *)malloc(4);
                h = s;
                free(s);
                s =  (uint8_t *)(SP); 
                return static_cast<uint16_t>(*s);
#           else
            char top;
#             if defined(__arm__)
                return static_cast<uint16_t>(&top - reinterpret_cast<char*>(sbrk(0)));
#             elif (defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151))
                return static_cast<uint16_t>(&top - __brkval);
#             else
                return static_cast<uint16_t>((__brkval) ? (&top - __brkval) : (&top - __malloc_heap_start));
#             endif
#           endif
        }
        uint8_t getId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_AUTO_FREE_MEM);
        }

    public:
        void go_init() {}
        bool go_presentation() {

            uint8_t id = getId();
            if (!presentSend(id, S_CUSTOM, "Unit.Memory.Free"))
              return false;
            if (!presentSend(id, V_CUSTOM))
              return false;
            return true;
        }
        void go_data(__attribute__ (( __unused__ )) uint16_t & cnt) {

            if (isAction) {
                isAction = false;
                reportMsg(getId(), V_CUSTOM, getFreeMem());
            }
        }
        bool go_data(const MyMessage & msg) {
            switch (msg.getType()) {
                case V_CUSTOM: {
                    if (msg.sensor == getId()) {
                        isAction = true;
                        return true;
                    }
                    break;
                }
            }
            return false;
        }
};

#endif
