
#include <Arduino.h>
#include <MySensors.h>

struct NodeStat
{
#   if (INTERNAL_LIVE_TEMP > 0)
#     define TEMP_328_CAL_OFFSET 334
#     define TEMP_328_FILTER_ALP 0.1
#     define ENABLE_SENSOR_TEMP 1
#   endif
#   if (INTERNAL_LIVE_RSSI > 0)
#     define ENABLE_SENSOR_RSSI 1
#   endif
#   if (defined(PIN_INTERNAL_LIVE_VOLT) && (PIN_INTERNAL_LIVE_VOLT > 0))
#     define ENABLE_SENSOR_VOLT 1
#   endif

    private:
        float temp = 0;
        uint16_t cnt = 0U;
        uint16_t volt = 0U;
        uint16_t rssi = 0U;
        uint16_t radio_quality() {
#           if defined(ENABLE_SENSOR_RSSI)
            int16_t t = transportGetReceivingRSSI(),
                    r = map(t, -85, -40, 0, 100);
            return static_cast<uint16_t>((r < 0) ? 0 : ((r > 100) ? 100 : r));
#           else
            return 0U;
#           endif
        }
        bool chipTemperature() {
#           if defined(ENABLE_SENSOR_TEMP)
            // ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
            ADMUX = 0xC8;
            ADCSRA |= _BV(ADSC);
            // ADCSRA |= _BV(ADEN) | _BV(ADSC);
            // ADCSRA |= _BV(ADEN);             

            while (bit_is_set(ADCSRA, ADSC));
            int16_t t_ = ((ADCL | (ADCH << 8)) - TEMP_328_CAL_OFFSET);
            float toff_ = ((TEMP_328_FILTER_ALP * t_) + ((1.0 - TEMP_328_FILTER_ALP) * temp));
            return ((temp != toff_) ? __extension__ ({temp = toff_; true;}) : false);
#           else
            return false;
#           endif
        }
        bool pinVolage() {
#           if defined(ENABLE_SENSOR_VOLT)
            int16_t volt_ = ((analogRead(PIN_INTERNAL_LIVE_VOLT) - 600) / 3);
            volt_ = ((volt_ < 0) ? 0 : ((volt_ > 100) ? 100 : volt_));
            return ((volt != volt_) ? __extension__ ({volt = volt_; true;}) : false);
#           else
            return false;
#           endif
        }

    public:
        void init() {
        }
        bool presentation() {
            
#           if defined(ENABLE_SENSOR_RSSI)
            if (!presentSend(INTERNAL_LIVE_RSSI, S_CUSTOM))
                return false;
#           endif
#           if defined(ENABLE_SENSOR_TEMP)
            if (!presentSend(INTERNAL_LIVE_TEMP, S_TEMP))
                return false;
#           endif
#           if defined(ENABLE_SENSOR_VOLT)
            if (!presentSend(INTERNAL_LIVE_VOLT, S_MULTIMETER))
                return false;
#           endif
            return true;
        }
        void data() {

            if (++cnt > 100)
                return;
            
            cnt = 0U;

#           if defined(ENABLE_SENSOR_VOLT)
            if (pinVolage()) {
                MyMessage msg(INTERNAL_LIVE_VOLT, V_VOLTAGE);
                send(msg.set(volt), true);
            }
#           endif

#           if defined(ENABLE_SENSOR_TEMP)
            if (chipTemperature()) {
                MyMessage msg(INTERNAL_LIVE_TEMP, V_TEMP);
                send(msg.set(temp, 1), true);
            }
#           endif

#           if defined(ENABLE_SENSOR_RSSI)
            uint16_t rssi_ = radio_quality();
            if (rssi == rssi_)
                return;
            
            rssi = rssi_;
            _sendRoute(
                build(
                    _msgTmp,
                    GATEWAY_ADDRESS,
                    INTERNAL_LIVE_RSSI,
                    C_SET,
                    V_VAR1,
                    false
                ).set(rssi)
            );
#           endif
        }
};
