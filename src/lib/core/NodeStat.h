#if !defined(__MY_SENSOR_STATISTIC_H)
#define __MY_SENSOR_STATISTIC_H 1

#include <Arduino.h>
#include <MySensors.h>

#if defined(MY_GATEWAY_ESP8266)
#  include <ESP.h>
#  include <ESP8266WiFi.h>
#endif

struct NodeStat
{
#   define POLL_TIMEOUT 2000U
#undef ENABLE_LIVE_SENSOR_ILLUMINATION

    private:
        int8_t temp = 0;
        uint8_t volt = 0U;
        int16_t rssi = 0;
        uint16_t illum = 0U;
        uint16_t cnt = POLL_TIMEOUT;
        bool chipTemperature() {
#           if defined(ENABLE_LIVE_SENSOR_TEMP)
            int8_t t_ = hwCPUTemperature();
            if (t_ == -127)
              return false;
            return ((temp != t_) ? __extension__ ({temp = t_; true;}) : false);
#           else
            return false;
#           endif
        }
        void radioQuality() {
#           if defined(ENABLE_LIVE_SENSOR_RSSI)
#             if defined(MY_GATEWAY_ESP8266)
                int16_t r_ = static_cast<int16_t>(WiFi.RSSI());
#             else
                int16_t r_ = transportGetSendingRSSI();
#             endif
            
            r_ = map(r_, -85, -40, 0, 100);
            r_ = ((r_ < 0) ? 0 : ((r_ > 100) ? 100 : r_));
            if (rssi != r_)
              rssi = r_;
#           endif
        }
        void chipVoltage() {
#           if defined(ENABLE_LIVE_SENSOR_VOLT)
#             if defined(MY_GATEWAY_ESP8266)
               int16_t v_ = ESP.getVcc();
#             else
                int16_t v_ = analogRead(INTERNAL_LIVE_VOLT_PIN) / 10;
#             endif

            v_ = ((v_ < 0) ? 0 : ((v_ > 100) ? 100 : v_));
            if (volt != v_)
              volt = static_cast<uint8_t>(v_);
#           endif
        }
        bool sensorIlluminate() {
#         if defined(ENABLE_LIVE_SENSOR_ILLUMINATION)
          uint16_t val = ((2500 / (
            (analogRead(INTERNAL_LIVE_ILLUMINATION_PIN) * 0.0048828125)
            - 500)) / 10);
          
          if (static_cast<int16_t>(val) < 0)
            return false;
          if (val == illum)
            return false;
          illum = val;
          return true;

#         else
          return false;
#         endif
        }

    public:
        void init() {
#           if defined(ENABLE_LIVE_SENSOR_VOLT)
#             if defined(MY_GATEWAY_ESP8266)
                ADC_MODE(ADC_VCC);
#             else
                analogReference(INTERNAL);
#             endif
#           endif
        }
        bool presentation() {

#           if defined(ENABLE_LIVE_SENSOR_TEMP)
            if (!presentSend(INTERNAL_LIVE_TEMP, S_TEMP))
                return false;
#           endif
#           if defined(ENABLE_LIVE_SENSOR_ILLUMINATION)
            if (!presentSend(INTERNAL_LIVE_ILLUMINATION_PIN, S_LIGHT_LEVEL))
                return false;
#           endif
            return true;
        }
        void data() {

#           if (!defined(ENABLE_LIVE_SENSOR_TEMP) && (!defined(ENABLE_LIVE_SENSOR_ILLUMINATION)))
            return;
#           else

            if ((++cnt < POLL_TIMEOUT) || (!isTransportReady()))
                return;
            cnt = 0U;

#           if defined(ENABLE_LIVE_SENSOR_ILLUMINATION)
            if (sensorIlluminate()) {
                MyMessage msg(INTERNAL_LIVE_ILLUMINATION, V_LIGHT_LEVEL);
                send(msg.set(illum), true);
            }
#           endif
#           if defined(ENABLE_LIVE_SENSOR_TEMP)
            if (chipTemperature()) {
                MyMessage msg(INTERNAL_LIVE_TEMP, V_TEMP);
                send(msg.set(temp), true);
            }
#           endif
#           endif
        }
        bool data(const MyMessage & msg) {
          PRINTF("-- STAT msg.sensor: %u, type: %u\n", msg.sensor, msg.type);
          if (msg.type == I_BATTERY_LEVEL) {
            chipVoltage();
            sendBatteryLevel(volt, false);
            return true;
          }
          if ((msg.type == V_VAR1) || (msg.type == V_VAR5) || (msg.type == I_SIGNAL_REPORT_REQUEST)) {
            radioQuality();
            PRINTF("-- rssi: %u\n", rssi);
            MY_CRITICAL_SECTION {
              char *buff = new char[18]{};
              (void) snprintf(buff, 17, "rssi: %d", rssi);
              //_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, 255, C_SET, I_SIGNAL_REPORT_RESPONSE, false).set(buff));
              _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, 255, C_SET, V_VAR5, false).set(rssi));
              //_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, 255, C_SET, I_SIGNAL_REPORT_RESPONSE, false).set(rssi));
              delete [] buff;
            }
            /*
            wait(500);
            _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, INTERNAL_LIVE_RSSI, C_SET, V_VAR5, false).set(buff));
            wait(500);
            _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, INTERNAL_LIVE_RSSI, C_SET, I_SIGNAL_REPORT_RESPONSE, false).set(rssi));
            wait(500);
            */
            return true;
          }
          return false;
        }
};

#endif
