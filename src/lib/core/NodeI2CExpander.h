#if !defined(__MY_SENSOR_I2C_EXPAND_H)
#define __MY_SENSOR_I2C_EXPAND_H 1

/* ------- I2C GPIO EXPANDER CONTROL ------- */

#  if defined(ENABLE_I2C_GPIO_EXPANDER)
#include <core/MyInclusionMode.h>
#include <PCF8574.h>

class NodeI2CExpander : public SensorInterface<NodeI2CExpander> {

    private:
        PCF8574 *expand;
        const uint8_t led_device[5] { 2U, 3U, 6U, 4U, 5U }; /* Color: W - (INFO), R(mys), E(mys), G(mys), R - (ERR) */
        const uint8_t btn_device[3] { 7U, 0U, 1U  }; // 1U, 0U - free

    public:
        NodeI2CExpander() {
            expand = new PCF8574(0x22, sda_PIN, scl_PIN); /* address, SDA, SDL */
            for (uint8_t i = 0U; i < __NELE(led_device); i++) {
                expand->pinMode(led_device[i], OUTPUT);
                expand->digitalWrite(led_device[i], HIGH);
            }
            for (uint8_t i = 0U; i < __NELE(btn_device); i++) {
                expand->digitalWrite(btn_device[i], HIGH);
                expand->pinMode(btn_device[i], INPUT);
            }
        }
        ~NodeI2CExpander() {
            delete expand;
        }
        void led(uint8_t & id, bool status) {
            if (id >= __NELE(led_device))
                return;
            expand->digitalWrite(led_device[id], ((status) ? LOW : HIGH));
        }
        bool btn(uint8_t & id) {
            if (id >= __NELE(btn_device))
                return false;
            return expand->digitalRead(btn_device[id]);
        }
        bool go_init() {
            expand->begin();
            return true;
        }
        bool go_presentation() {
            return true;
        }
        void go_data(__attribute__ (( __unused__ )) uint16_t&) {
        }
        bool go_data(__attribute__ (( __unused__ )) const MyMessage & msg) {
            return false;
        }
        void myRxLed(uint8_t & state) {
            expand->digitalWrite(led_device[1], ((state) ? HIGH : LOW));
        }
        void myTxLed(uint8_t & state) {
            expand->digitalWrite(led_device[2], ((state) ? HIGH : LOW));
        }
        void myErrLed(uint8_t & state) {
            expand->digitalWrite(led_device[3], ((state) ? HIGH : LOW));
        }
        void errorLed(uint8_t state) {
            expand->digitalWrite(led_device[4], ((state) ? HIGH : LOW));
        }
        void infoLed(uint8_t & state) {
            expand->digitalWrite(led_device[0], ((state) ? LOW : HIGH));
        }
        bool incluseBtn() {
            return (expand->digitalRead(btn_device[0]) == LOW);
        }
};

#  endif
#endif
