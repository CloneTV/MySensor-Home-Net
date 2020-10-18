
#if !defined(__MY_SENSOR_NODE_CMD_H)
#define __MY_SENSOR_NODE_CMD_H 1

#  if defined(ENABLE_SENSOR_RF433)
#  include "ESP8266_RF433T.h"
#  include "Int64String.h"

#  if defined(ENABLE_SENSOR_IR)
#    include <IRremoteESP8266.h>
#    include <IRrecv.h>
#  endif

#  if defined(USE_MY_CONTROLLER_GROUP)
#    include <ESP8266HTTPClient.h>
#  endif

/* ------- BUILD-IN NODE GW COMMAND ------- */

#  define IDX_Change 0
#  define IDX_Reboot 1

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
        bool isAction[2] = {
            false,
            false
        };
        uint8_t curId = 0U;
        unsigned long startOffset = 0U;
        ESP8266_RF433T<> *rf433;
#       if defined(ENABLE_SENSOR_IR_RECEIVE)
        IRrecv *irRcv;
        decode_results irdata{};
#       endif
#       if (defined(CMD_GROUP_MY_CONTROLLER) && (CMD_GROUP_MY_CONTROLLER > 0))
        const char *uri_on = CMD_GROUP_URI_ON;
        const char *uri_off = CMD_GROUP_URI_OFF;
        const char *payload_65 = "[65]";
        const char *payload_97 = "[97]";
#       endif
        EventVirtualSwitch ev[18] = {
            { 1U,   LOW, LOW, 0U, 5180193U, 16712445LLU },     // (0)  5 // switch 1 5180193/5180194
            { 2U,   LOW, LOW, 0U, 5180194U, 16761405LLU },     // (1)  6
            { 3U,   LOW, LOW, 0U, 750369U,  16769055LLU },     // (2)  7 // switch 2 750369/750370
            { 4U,   LOW, LOW, 0U, 750370U,  16754775LLU },     // (3)  8
            { 5U,   LOW, LOW, 0U, 3276321U, 16748655LLU },     // (4)  9 // switch 3 3276321/3276322 (clash 5180193 ?)

#           if (defined(CMD_GROUP_MY_CONTROLLER) && (CMD_GROUP_MY_CONTROLLER > 0))
#           define SCENE_START_ID 100U
            { 111U,  LOW, LOW, 0U, 0U, 16726215LLU },          // (5)  OK
            { 112U,  LOW, LOW, 0U, 0U, 16738455LLU },          // (6)  * scenario On
            { 113U,  LOW, LOW, 0U, 0U, 0U },                   // (7)  - empty -
#           else
#           define SCENE_START_ID 10U
            { 10U,  LOW, LOW, 0U, 0U, 16726215LLU },           // (5)  OK
            { 11U,  LOW, LOW, 0U, 0U, 16738455LLU },           // (6)  * scenario On
            { 12U,  LOW, LOW, 0U, 0U, 0U },                    // (7)  virtual 'All Off'
#           endif
            { 101U, LOW, LOW, 0U, 0U, 16756815LLU },           // (8)  # scenario Off
            { 106U, LOW, LOW, 0U, 0U, 16750695LLU },           // (9)  0

            { 102U, LOW, 4U,  0U, 0U, 16753245LLU },           // (10)  1
            { 103U, LOW, 3U,  0U, 0U, 16736925LLU },           // (11) 2
            { 104U, LOW, 0U,  0U, 0U, 16769565LLU },           // (12) 3
            { 105U, LOW, 1U,  0U, 0U, 16720605LLU },           // (13) 4

            { 107U, LOW, LOW, 0U, 0U, 16718055LLU },           // (14) ^ up
            { 108U, LOW, LOW, 0U, 0U, 16716015LLU },           // (15) <
            { 109U, LOW, LOW, 0U, 0U, 16734885LLU },           // (16) >
            { 110U, LOW, LOW, 0U, 0U, 16730805LLU }            // (17) ^ down
        };
        template<typename T>
        void mqttSend(uint8_t node, uint8_t sensor, mysensors_data_t type, T val) {

            if (WiFi.status() != WL_CONNECTED)
                return;

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
            isAction[IDX_Change] = ((isAction[IDX_Change]) ? isAction[IDX_Change] : true);
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
                case 16726215LLU:       /* scenario OnAll (OK) Group (97)/Scene 10 */
                case 16750695LLU:       /* scenario OffAll (0) Group (97)/Scene 10 */
                case 16756815LLU:       /* scenario Off (#) Group (65)/Scene 11 */
                case 16738455LLU: {     /* scenario On (*) Group (65)/Scene 11 */
                    if (WiFi.status() != WL_CONNECTED)
                        break;
                    
#                   if (defined(CMD_GROUP_MY_CONTROLLER) && (CMD_GROUP_MY_CONTROLLER > 0))
                    /* Group  65, 97 */
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
#                   else
                    /* Scene  10,11 */
                    uint8_t scid, scstatus;
                    switch (ev[idx].irid) {
                        case 16726215LLU: { scid = 5U; scstatus = HIGH; break; } /* Scene 10 On */
                        case 16750695LLU: { scid = 5U; scstatus = LOW; break;  } /* Scene 10 Off */
                        case 16738455LLU: { scid = 6U; scstatus = HIGH; break; } /* Scene 11 On */
                        case 16756815LLU: { scid = 6U; scstatus = LOW; break;  } /* Scene 11 Off */
                        default: return;
                    }
                    ev[scid].s = scstatus;
                    setEvent(scid, scstatus);
#                   endif
                    break;
                }
                default:
                    break;
            }
        }
        uint8_t getIrRcvId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_IR_RECEIVE);
        }
        uint8_t getRebootBtnId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_CMD_REBOOT);
        }

    public:
        NodeCommand() {
            rf433 = new ESP8266_RF433T<>();
#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            pinMode(INTERNAL_LIVE_IR_RECEIVE_PIN, INPUT);
            irRcv = new IRrecv(INTERNAL_LIVE_IR_RECEIVE_PIN);
#           endif
        }
        ~NodeCommand() {
            delete rf433;
#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            delete irRcv;
#           endif
        }
        void lightReceive(int16_t & level) {
            
        }
        bool go_init() {
            rf433->transmit(true);
            for (uint8_t i = 0U; i < __NELE(ev); i++) {
                if (ev[i].n >= 100U)
                    break;
                ev[i].s = loadState(ev[i].n);
            }
#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            irRcv->enableIRIn();
#           endif
            return true;
            startOffset = millis() + 60000U;
        }
        bool go_presentation() {
            if (WiFi.status() != WL_CONNECTED)
                return false;

            for (uint8_t i = 0U; i < __NELE(ev); i++) {
                if (ev[i].n >= 100U)
                    break;
                if ((ev[i].rfid) && (ev[i].n < SCENE_START_ID)) {
                    if (!presentSend(ev[i].n, S_BINARY, "Relay.Virtual.Lights"))
                        return false;
                    if (!presentSend(ev[i].n, V_STATUS))
                        return false;
                } else if (ev[i].n >= SCENE_START_ID) {
                    if (!presentSend(ev[i].n, S_SCENE_CONTROLLER, "Scene.Virtual.Switch"))
                        return false;
                    if (!presentSend(ev[i].n, V_SCENE_ON))
                        return false;
                    ev[i].s = LOW;
                    ev[i].e = HIGH;
                }
            }
            uint8_t id = getRebootBtnId();
            if (!presentSend(id, S_BINARY, "Sys.Reboot"))
              return false;
            if (!presentSend(id, V_STATUS))
              return false;

#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            id = getIrRcvId();
            if (!presentSend(id, S_IR))
              return false;
            if (!presentSend(id, V_IR_RECEIVE))
              return false;
#           endif
            return true;
        }
        void go_data(__attribute__ (( __unused__ )) uint16_t&) {

#           if defined(ENABLE_SENSOR_IR_RECEIVE)
            if (irRcv->decode(&irdata)) {
                /// NEC only, ToDo: more standarts..
                if ((irdata.decode_type == NEC) && (irdata.bits == 32U) && (irdata.value != -1ULL)) {
                    
                    if (WiFi.status() == WL_CONNECTED) {
                        String s = Int64ToString(irdata.value, HEX, true);
                        reportMsg(
                            getIrRcvId(),
                            V_IR_RECEIVE,
                            s.c_str()
                        );
                    }

                    uint8_t idx = checkIrCode(irdata.value);
                    if (idx != SENSOR_ID_NONE)
                        setEvent(idx, ((ev[idx].s == LOW) ? HIGH : LOW));
                }
                irRcv->resume();
            }
#           endif
            if (isAction[IDX_Change]) {
                isAction[IDX_Change] = false;
                
                for (uint8_t i = 0U; i < __NELE(ev); i++) {
                    if (ev[i].n >= 100U)
                        break;
                    
                    if (ev[i].e != HIGH)
                        continue;
                    ev[i].e = LOW;
                    mysensors_data_t tdata;

                    if ((ev[i].rfid) && (ev[i].n < SCENE_START_ID)) {
                        tdata = V_STATUS;
                        rf433->send(ev[i].rfid, 24);

                    } else if (ev[i].n >= SCENE_START_ID) {
                        tdata = V_SCENE_ON;
                    } else {
                        continue;
                    }
                    saveState(ev[i].n, ev[i].s);
                    if (WiFi.status() == WL_CONNECTED)
                        reportMsg(ev[i].n, tdata, static_cast<bool>(ev[i].s));
                }
            }
            if (isAction[IDX_Reboot]) {
                isAction[IDX_Reboot] = false;
                PRINTF("-- Reboot received! %d = (),  %lu/%lu\n", WiFi.status(), WL_CONNECTED, startOffset, millis());

                if (WiFi.status() == WL_CONNECTED)
                    reportMsg(getRebootBtnId(), V_STATUS, false);
                if (startOffset >= millis())
                    return;
                delay(500);
                ESP.restart();
                while(true) { yield(); };
            }
        }
        bool go_data(const MyMessage & msg) {
            if (msg.getSender() != 0U)
                return false;

            uint8_t idx;
            switch (msg.getType()) {
                case V_STATUS:
                case V_SCENE_ON:
                case V_SCENE_OFF: {
                    if ((msg.sensor == getRebootBtnId()) &&
                        (msg.getType() == V_STATUS)) {
                        isAction[IDX_Reboot] = msg.getBool();
                        break;
                    }
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

#  undef IDX_Change
#  undef IDX_Reboot
#  endif
#endif
