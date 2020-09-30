
#if !defined(__MY_SENSOR_NODE_CMD_H)
#define __MY_SENSOR_NODE_CMD_H 1

#  if defined(ENABLE_SENSOR_RF433)
#  define RCSwitchDisableReceiving 1
#  include <RCSwitch.h>
#  include "Int64String.h"
RCSwitch rfSwitch = RCSwitch();

#  if defined(ENABLE_SENSOR_IR)
#    include <IRremoteESP8266.h>
#    include <IRrecv.h>
#  endif

#include <ESP8266HTTPClient.h>

/* ------- BUILD-IN NODE GW COMMAND ------- */

/*
    EventVirtual.n = sensor id
    EventVirtual.s = sensor state
    EventVirtual.e = event enable/disable or id >= 100 real sensor id
    EventVirtual.d = dimmer value
    EventVirtual.rfid = 433/315Mhz switch id
    EventVirtual.irid = IR code maped to..
 */
struct EventVirtualSwitch {
  uint8_t n, s, e, d;
  uint32_t rfid;
  uint64_t irid;
} __attribute__((packed));

class NodeCommand : public SensorInterface<NodeCommand> {
    private:
        bool isAction = false;
        uint8_t curId = 0U;
#       if defined(ENABLE_SENSOR_IR_RECEIVE)
        IRrecv *irRcv;
        decode_results irdata{};
#       endif
        const char *uri_on = CMD_GROUP_URI_ON;
        const char *uri_off = CMD_GROUP_URI_OFF;
        const char *payload_65 = "[65]";
        const char *payload_97 = "[97]";
        EventVirtualSwitch ev[17] = {
            { 1U,   LOW, LOW, 0U, 5180193U, 16712445LLU },    // 5
            { 2U,   LOW, LOW, 0U, 5180194U, 16761405LLU },    // 6
            { 3U,   LOW, LOW, 0U, 0U, 16769055LLU },          // 7
            { 4U,   LOW, LOW, 0U, 0U, 16754775LLU },          // 8
            { 5U,   LOW, LOW, 0U, 0U, 16748655LLU },          // 9

            { 102U, LOW, 4U,  0U, 0U, 16753245LLU },          // 1
            { 103U, LOW, 3U,  0U, 0U, 16736925LLU },          // 2
            { 104U, LOW, 0U,  0U, 0U, 16769565LLU },          // 3
            { 105U, LOW, 1U,  0U, 0U, 16720605LLU },          // 4

            { 101U, LOW, LOW, 0U, 0U, 16750695LLU },          // 0
            { 106U, LOW, LOW, 0U, 0U, 16726215LLU },          // OK
            { 107U, LOW, LOW, 0U, 0U, 16718055LLU },          // ^ up
            { 108U, LOW, LOW, 0U, 0U, 16716015LLU },          // <
            { 109U, LOW, LOW, 0U, 0U, 16734885LLU },          // >
            { 110U, LOW, LOW, 0U, 0U, 16730805LLU },          // ^ down
            { 111U, LOW, LOW, 0U, 0U, 16738455LLU },          // * scenario On
            { 112U, LOW, LOW, 0U, 0U, 16756815LLU }           // #  scenario Off
        };
        template<typename T>
        void mqttSend(uint8_t node, uint8_t sensor, mysensors_data_t type, T val) {

            MyMessage msg(sensor, type);
            msg.sender = node;
            msg.setDestination(node);
            msg.setSensor(sensor);
            msg.setCommand(C_SET);
            msg.setRequestEcho(0);
            msg.setType(type);
            msg.set(val);
            (void) gatewayTransportSendToSubsribe(msg, false);
        }
        uint8_t checkId(uint8_t id) {
            for (uint8_t i = 0U; i < __NELE(ev); i++) {
                if (ev[i].n >= 100U)
                    break;
                if (ev[i].n == id)
                    return i;
            }
            return SENSOR_ID_NONE;
        }
        bool checkSensorIsRange() {
            if ((!curId) || (curId >= __NELE(ev)) || (!ev[curId].e))
                return false;
            switch (ev[curId].n) {
                case 102U:
                case 103U: return true;
                default: return false;
            }
        }
        uint8_t checkIrCode(uint64_t & ir) {
            for (uint8_t i = 0U; i < __NELE(ev); i++) {
                if ((ev[i].irid > 0ULL) && (ev[i].irid == ir))
                    return i;
            }
            return SENSOR_ID_NONE;
        }
        void setEvent(uint8_t & idx, uint8_t val = LOW) {
            if (ev[idx].n >= 100U) {
                setMqttEvent(idx, val);
                return;
            }
            curId = idx;
            ev[idx].s = val;
            ev[idx].e = HIGH;
            isAction = ((isAction) ? isAction : true);
        }
        void setMqttEvent(uint8_t & idx, uint8_t val = LOW) {
            switch (ev[idx].irid) {
                case 16730805LLU:       /* down - 22/3/1/0/3 - 22/4/1/0/3 */
                case 16716015LLU: {     /* left */
                    if ((!checkSensorIsRange()) || (ev[curId].d == 0U))
                        return;

                    ev[curId].d = ((ev[curId].d > 10U) ? (ev[curId].d - 10U) : 0U);
                    if (ev[curId].d == 0U) {
                        ev[curId].s = LOW;
                        mqttSend(22U, ev[curId].e, V_STATUS, ev[curId].s);
                    } else {
                        mqttSend(22U, ev[curId].e, V_PERCENTAGE, static_cast<uint16_t>(ev[curId].d));
                    }
                    return;
                }
                case 16718055LLU:       /* up - 22/3/1/0/3 - 22/4/1/0/3 */
                case 16734885LLU: {     /* right */
                    if ((!checkSensorIsRange()) || (ev[curId].d == 100U))
                        return;

                    ev[curId].d = ((ev[curId].d < 90U) ? (ev[curId].d + 10U) : 100U);
                    mqttSend(22U, ev[curId].e, V_PERCENTAGE, static_cast<uint16_t>(ev[curId].d));
                    return;
                }
                case 16753245LLU:       /* 22/4/1/0/2 */
                case 16736925LLU: {     /* 22/3/1/0/2 */
                    curId = idx;
                    ev[idx].s = val;
                    ev[idx].d = ((val) ? 100U : 0U);
                    mqttSend(22U, ev[idx].e, V_STATUS, val);
                    break;
                }
                case 16769565LLU:       /* 22/0/1/0/2 */
                case 16720605LLU: {     /* 22/1/1/0/2 */
                    curId = idx;
                    ev[idx].s = val;
                    mqttSend(22U, ev[idx].e, V_STATUS, val);
                    break;
                }
                case 16726215LLU:       /* scenario OnAll (OK) Group (97) */
                case 16750695LLU:       /* scenario OffAll (0) Group (97) */
                case 16756815LLU:       /* scenario Off (#) Group (65) */
                case 16738455LLU: {     /* scenario On (*) Group (65) */
                    HTTPClient http;
                    const char *uri, *payload;
                    switch (ev[idx].irid) {
                        case 16756815LLU: {
                            payload = payload_65;
                            uri = uri_off;
                            break;
                        }
                        case 16738455LLU: {
                            payload = payload_65;
                            uri = uri_on;
                            break;
                        }
                        case 16750695LLU: {
                            payload = payload_97;
                            uri = uri_off;
                            break;
                        }
                        case 16726215LLU: {
                            payload = payload_97;
                            uri = uri_on;
                            break;
                        }
                        default: return;
                    }
                    http.begin(CMD_GROUP_HOST, CMD_GROUP_PORT, uri);
                    http.addHeader(F("Content-Type"), F("application/json"));
                    http.addHeader(F("Authorization"), "Basic " CMD_GROUP_AUTH, true);
                    (void) http.POST(payload);
                    break;
                }
                default:
                    break;
            }
        }
        uint8_t getIrRcvId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_IR_RECEIVE);
        }

    public:
        NodeCommand() {
#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            pinMode(INTERNAL_LIVE_IR_RECEIVE_PIN, INPUT);
            irRcv = new IRrecv(INTERNAL_LIVE_IR_RECEIVE_PIN);
#           endif
        }
        ~NodeCommand() {
#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            delete irRcv;
#           endif
        }
        void lightReceive(int16_t & level) {
            
        }
        bool go_init() {
            rfSwitch.enableTransmit(INTERNAL_RF433_PIN);
            for (uint8_t i = 0U; i < __NELE(ev); i++) {
                if (ev[i].n >= 100U)
                    break;
                if (ev[i].rfid)
                    ev[i].s = loadState(ev[i].n);
            }
#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            irRcv->enableIRIn();
#           endif
            return true;
        }
        bool go_presentation() {
            for (uint8_t i = 0U; i < __NELE(ev); i++) {
                if (ev[i].n >= 100U)
                    break;
                if (!presentSend(ev[i].n, S_BINARY, "Relay.Virtual.Lights"))
                    return false;
                if (!presentSend(ev[i].n, V_STATUS))
                    return false;
            }
#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            uint8_t rid = getIrRcvId();
            if (!presentSend(rid, S_IR))
              return false;
            if (!presentSend(rid, V_IR_RECEIVE))
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
                    uint8_t idx = checkIrCode(irdata.value);
                    if (idx != SENSOR_ID_NONE)
                        setEvent(idx, ((ev[idx].s == LOW) ? HIGH : LOW));
                }
                irRcv->resume();
            }
#           endif
            if (isAction) {
                isAction = false;
                
                for (uint8_t i = 0U; i < __NELE(ev); i++) {
                    if (ev[i].n >= 100U)
                        break;
                    if (ev[i].e == HIGH) {
                        if (ev[i].rfid)
                            rfSwitch.send(ev[i].rfid, 24);
                        ev[i].e = LOW;
                        saveState(ev[i].n, ev[i].s);
                        reportMsg(ev[i].n, V_STATUS, static_cast<bool>(ev[i].s));
                    }
                }
            }
        }
        bool go_data(const MyMessage & msg) {
            if (msg.getSender() != 0U)
                return false;

            uint8_t idx;
            switch (msg.getType()) {
                case V_STATUS: {
                    idx = checkId(msg.sensor);
                    if (idx == SENSOR_ID_NONE)
                        return false;
                    
                    setEvent(idx, msg.getByte());
                    break;
                }
                case V_IR_RECEIVE: {
                    String s(msg.getString());
                    uint64_t code = StringToInt64(s);
                    idx = checkIrCode(code);
                    if (idx == SENSOR_ID_NONE)
                        return false;
                    
                    setEvent(idx, ((ev[idx].s == LOW) ? HIGH : LOW));
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
