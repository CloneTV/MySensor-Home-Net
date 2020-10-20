#if !defined(__MY_SENSOR_SOIL_H)
#define __MY_SENSOR_SOIL_H 1

# if defined(ENABLE_SENSOR_SOIL)
#  define PIN_index_1 0
#  define PIN_index_2 1
#  define PIN_index_NRF 2
#  define PIN_index_A 3

#  define POLL_WAIT_SECONDS 600U
#  define IDX_Enable 0
#  define IDX_Change 1
#  define IDX_Rfinit 2


class NodeMoisture : public SensorInterface<NodeMoisture> {
    private:
        bool isAction[3] = { true, false, false };
        uint16_t lastval = 0U;
        const uint8_t pins_[4] = { SOIL_SENSOR_PINS };
        uint8_t getSoilId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_SOIL);
        }
        uint8_t getInfoId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_INFO);
        }
        uint8_t getVoltId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_VOLT);
        }
        uint16_t readData() {
            uint8_t  cnt = 0U;
            uint16_t val = 0U;
            analogReference(DEFAULT);
            for(uint8_t i = 0U; i < 10; i++) {
                digitalWrite(pins_[PIN_index_2], LOW);
                digitalWrite(pins_[PIN_index_1], HIGH);
                wait(1);
                digitalWrite(pins_[PIN_index_1], LOW);
                digitalWrite(pins_[PIN_index_2], HIGH);
                uint16_t v = analogRead(pins_[PIN_index_A]);
                if (i > 4U) {
                    val += v;
                    cnt++;
                }
                wait(1);
            }
            digitalWrite(pins_[PIN_index_1], LOW);
            digitalWrite(pins_[PIN_index_2], LOW);
            val /= cnt;
            return val;
        }
        float readVcc() {
            long result;
            ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
            delay(2);
            ADCSRA |= _BV(ADSC);
            while (bit_is_set(ADCSRA,ADSC));
            result = ADCL;
            result |= ADCH << 8;
            result = (1100L * 1023)/result;
            return (((float)result)/1000);
        }

    public:
        NodeMoisture() {

        }
        void enable() {
            isAction[IDX_Enable] = true;
        }
        bool ischange() {
            if (isAction[IDX_Change]) {
                isAction[IDX_Change] = false;
                return true;
            }
            return false;
        }
        bool rfbegin(bool b = true) {
            /* On NRF24 */
            if (digitalRead(pins_[PIN_index_NRF]) == LOW) {
                digitalWrite(pins_[PIN_index_NRF], HIGH);
                while (b) {
                    if (isTransportReady())
                        break;
                    PRINTLN("--- isTransportReady == false");
                    wait(500);
                }
            }
            if (!isAction[IDX_Rfinit]) {
                if (b)
                    isAction[IDX_Rfinit] = isTransportReady();
                else
                    isAction[IDX_Rfinit] = true;
            }
            return isAction[IDX_Rfinit];
        }
        void rfend() {
            /* Off NRF24 */
            if (digitalRead(pins_[PIN_index_NRF]) == HIGH)
                digitalWrite(pins_[PIN_index_NRF], LOW);
            if (isAction[IDX_Rfinit])
                isAction[IDX_Rfinit] = false;
        }
        bool go_init() {
#           if defined(MY_GATEWAY_ESP8266)
            ADC_MODE(ADC_VCC);
#           elif defined(__AVR_ATmega2560__)
            analogReference(INTERNAL1V1);
#           else
            analogReference(INTERNAL);
#           endif

            for (uint8_t i = 0U; i < (__NELE(pins_) - 1U); i++) {
                pinMode(pins_[i], OUTPUT);
                digitalWrite(pins_[i], LOW);
            }
            pinMode(MY_PASSIVE_BTN_PIN, INPUT_PULLUP);
            return rfbegin(false);
        }
        bool go_presentation() {
            do {
                if ((!isTransportReady()) && (!rfbegin()))
                    break;
                if (!presentSend(getSoilId(), S_MOISTURE, "Int.Soil"))
                    break;
                if (!presentSend(getSoilId(), V_LEVEL))
                    break;
                if (!presentSend(getInfoId(), S_INFO, "Info.Soil"))
                    break;
                if (!presentSend(getInfoId(), V_TEXT))
                    break;
                if (!presentSend(getVoltId(), S_MULTIMETER, "Int.Bat.Volt"))
                    break;
                if (!presentSend(getVoltId(), V_VOLTAGE))
                    break;
                return true;
            } while (0);
            return false;
        }
        void go_data(const uint16_t & cnt) {
            if ((isAction[IDX_Enable]) || ((cnt % POLL_WAIT_SECONDS) == 0)) {
                if (isAction[IDX_Enable])
                    isAction[IDX_Enable] = false;

                if ((!isTransportReady()) && (!rfbegin()))
                    return;

                float v = readVcc();
                {
                    reportMsg(getVoltId(), V_VOLTAGE, v, 2U);
                    wait(100);
                    uint16_t volt = map(v, 1.6, 3.3, 0, 100);
                    sendBatteryLevel(volt, false);
                }
                int32_t m = readData();
                {
                    if (lastval == m)
                        return;
                    lastval = m;
                    m = (0.000001 * m * m * m * -1);
                    m += (m * m * 0.001649);
                    m += ((m * -0.686978) + 121.559157);
                    m = (1024U - m);
                    m = map(m, 0, 1024, 0, 100);
                    isAction[IDX_Change] = true;
                    reportMsg(getSoilId(), V_LEVEL, m);
                }
            }
        }
        bool go_data(const MyMessage & msg) {
            switch (msg.getType()) {
                case V_LEVEL: {
                    if (msg.sensor != getSoilId())
                        return false;
                    break;
                }
                case V_VOLTAGE: {
                    if (msg.sensor != getVoltId())
                        return false;
                    break;
                }
                case V_TEXT: {
                    if (msg.sensor != getInfoId())
                        return false;
                    return true;
                }
                default:
                    return false;
            }
            isAction[IDX_Enable] = true;
            return true;
        }
};
#  undef PIN_index_A
#  undef PIN_index_1
#  undef PIN_index_2
#  undef PIN_index_NRF
#  undef POLL_WAIT_SECONDS
#  undef IDX_Enable
#  undef IDX_Change
#  undef IDX_Rfinit
# endif
#endif
