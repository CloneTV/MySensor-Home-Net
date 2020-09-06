
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <IRsend.h>
#include <IRrecv.h>
#include "Int64String.h"
#include "Gateway.h"

struct IrControl {
    private:
        IRrecv *irRcv;
        IRsend *irSnd;
        decode_results irdata{};
        //
        void sendData(String & val, const uint8_t sensorId, const mysensors_data_t dataType) {
            MyMessage msg(sensorId, dataType);
            send(msg.set(val.c_str()));
            msg.clear();
        }

    public:
        ~IrControl() {
            delete irRcv;
            delete irSnd;
        }
        IrControl() {
            irRcv = new IRrecv(PIN_SENSOR_IR_RECV);
            irSnd = new IRsend(PIN_SENSOR_IR_SEND); //, true, true);
        }
        void init() {
            irRcv->enableIRIn();
            irSnd->enableIROut(38000U);
            //irSnd->begin();
            //
            //send(msgIRSend.set(0));
            //send(msgIRRecv.set(0));

        }
        void toString() {
#         if (defined(MY_DEBUG) && !defined(MY_DISABLED_SERIAL))
          PRINTF("--- Type: %u\n", irdata.decode_type);
          PRINTF("--- Bits: %u\n", irdata.bits);
          PRINT("--- Address: ");
          PRINT2(irdata.address, HEX);
          PRINTF("\n--- Length: %u\n", irdata.rawlen);
          serialPrintUint64(irdata.value, HEX);
          PRINTF(" --- Code: %ull\n", irdata.value);

          if (!irdata.rawlen)
            return;

          PRINT("--- Raw Data: { ");
          for (uint16_t i = 1; i < irdata.rawlen; i++) {
            if (i % 100 == 0)
              yield();
            if (i & 1) {
              PRINTF("%u", irdata.rawbuf[i] * kRawTick);
            } else {
              PRINTF(", %u", irdata.rawbuf[i] * kRawTick);
            }
          }
          PRINT(" }\n");
#         endif
        }

        void data() {

          if (irRcv->decode(&irdata)) {
            // NEC only, ToDo: more standarts..
            if ((irdata.decode_type == NEC) && (irdata.bits == 32U) && (irdata.value != -1ULL)) {
              String s = Int64ToString(irdata.value, HEX, true);
              sendData(s, NODE_SENSOR_IR_RECV, V_IR_RECEIVE);
              toString();
            }
            irRcv->resume();
          }
        }

        void data(const MyMessage &msg) {
          if (msg.sensor != NODE_SENSOR_IR_SEND)
            return;
          
          const char *s = msg.getString();
          if (s == nullptr)
            return;

          uint64_t val = StringToInt64(s);
          if ((val == -1ULL) || (val == 0ULL))
            return;
            
          irSnd->sendNEC(val, 32);
          irRcv->enableIRIn();
        }
};
