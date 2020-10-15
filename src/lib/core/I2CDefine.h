#if !defined(I2C_DEFINE_H)
#define I2C_DEFINE_H

/*
Uno, Ethernet, Nano  A4 (SDA), A5 (SCL)
Mega2560, Due        20 (SDA), 21 (SCL)
Leonardo             2  (SDA), 3 (SCL)
ESP2866              4  (SDA), 5  (SCL), 14, 15
*/

#  if (defined(ESP8266) || defined(ESP32))
#    define DEF_SCL 5
#    define DEF_SDA 4
#  elif (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
#    define DEF_SCL 21
#    define DEF_SDA 20
#  elif (defined(__AVR_ATmega328P__) || \
        defined(__AVR_ATmega328__)  || \
        defined(__AVR_ATmega168A__) || \
        defined(__AVR_ATmega168__)  || \
        defined(__AVR_ATmega168P__) || \
        defined(__AVR_ATmega165P__) || \
        defined(__AVR_ATmega169A__) || \
        defined(__AVR_ATmega169__))
#    define DEF_SCL A5
#    define DEF_SDA A4
#  elif defined(__AVR_ATmega32U4__)
#    define DEF_SCL 3
#    define DEF_SDA 2
#  elif (defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__))
#    define DEF_SCL 0 // PB0
#    define DEF_SDA 2 // PB2
#  elif (defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__))
#    define DEF_SCL 0 // PB0??
#    define DEF_SDA 2 // PB2??
#  elif (defined(__AVR_AT90USB82__) || defined(__AVR_AT90USB162__) || defined(__AVR_ATmega32U2__) || defined(__AVR_ATmega16U2__) || defined(__AVR_ATmega8U2__))
#    error "AVR_AT90U - don't understand I2C pin.."
#  endif

static const uint8_t scl_PIN = DEF_SCL;
static const uint8_t sda_PIN = DEF_SDA;

#endif
