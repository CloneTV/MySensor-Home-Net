#if !defined(__MY_SENSOR_IR_CONTROL_H)
#define __MY_SENSOR_IR_CONTROL_H 1

/* ------- IR SEND RECEIVE CONTROL ------- */

#  if defined(ENABLE_SENSOR_IR)

#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <IRsend.h>
#include <IRrecv.h>
#include "Int64String.h"

class NodeIrControl : public SensorInterface<NodeIrControl> {

    private:

#       if defined(ENABLE_SENSOR_IR_SEND)
        bool isAction = false;
        uint64_t valSend = 0;
        IRsend *irSnd;
#       endif
#       if defined(ENABLE_SENSOR_IR_RECEIVE)
        IRrecv *irRcv;
        decode_results irdata{};
#       endif

        uint8_t getIrRcvId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_IR_RECEIVE);
        }
        uint8_t getIrSndId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_IR_SEND);
        }

    public:
        NodeIrControl() {
#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            pinMode(INTERNAL_LIVE_IR_RECEIVE_PIN, INPUT); // INPUT_PULLUP
            irRcv = new IRrecv(INTERNAL_LIVE_IR_RECEIVE_PIN);
#           endif
#           if defined(ENABLE_SENSOR_IR_SEND)
            pinMode(INTERNAL_LIVE_IR_SEND_PIN, OUTPUT);
            irSnd = new IRsend(INTERNAL_LIVE_IR_SEND_PIN); //, true, true);
#           endif
        }
        ~NodeIrControl() {
#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            delete irRcv;
#           endif
#           if defined(ENABLE_SENSOR_IR_SEND)
            delete irSnd;
#           endif
        }
        bool go_init() {
#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            irRcv->enableIRIn();
#           endif
#           if defined(ENABLE_SENSOR_IR_SEND)
            irSnd->enableIROut(38000U);
#           endif
            return true;
        }
        bool go_presentation() {
          
            /*
            PRINTLN("NODE IR | presentation");
            */
#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            uint8_t rid = getIrRcvId();
            if (!presentSend(rid, S_IR))
              return false;
            if (!presentSend(rid, V_IR_RECEIVE))
              return false;
#           endif
#           if defined(ENABLE_SENSOR_IR_SEND)
            uint8_t sid = getIrSndId();
            if (!presentSend(sid, S_IR))
              return false;
            if (!presentSend(sid, V_IR_SEND))
              return false;
#           endif
            return true;
        }
        void go_data(__attribute__ (( __unused__ )) uint16_t&) {

#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            if (irRcv->decode(&irdata)) {
                /// NEC only, ToDo: more standarts..
                if ((irdata.decode_type == NEC) && (irdata.bits == 32U) && (irdata.value != -1ULL)) {
                    String s = Int64ToString(irdata.value, HEX, true);
                    reportMsg(
                       getIrRcvId(),
                       V_IR_RECEIVE,
                       s.c_str()
                    );
                }
                irRcv->resume();
            }
#           endif
#           if defined(ENABLE_SENSOR_IR_SEND)
            if (isAction) {
                isAction = false;
                if ((valSend == -1ULL) || (valSend == 0ULL))
                    return;
                
                irSnd->sendNEC(valSend, 32);
                valSend = 0;
#               if defined(ENABLE_SENSOR_IR_RECEIVE)
                irRcv->enableIRIn();
#               endif
            }
#           endif
        }
        bool go_data(const MyMessage & msg) {

            switch (msg.getType()) {
                case V_IR_SEND: {
                    if (msg.sensor != getIrSndId())
                        return false;

#                   if defined(ENABLE_SENSOR_IR_SEND)
                    const char *s = msg.getString();
                    if (s == nullptr)
                        break;
                    valSend = StringToInt64(s);
                    isAction = true;
#                   endif

                    break;
                }
                case V_IR_RECEIVE: {
                    if (msg.sensor != getIrRcvId())
                        return false;
                    break;
                }
                default:
                    return false;
            }
            return true;
        }
};

#  endif
#endif
