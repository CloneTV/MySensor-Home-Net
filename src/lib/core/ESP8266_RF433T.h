/*
    Origin:
      RCSwitch - Arduino libary for remote control outlet switches
      Copyright (c) 2011 Suat Özgür.  All right reserved.

      Contributors:
      - Andre Koehler / info(at)tomate-online(dot)de
      - Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
      - Skineffect / http://forum.ardumote.com/viewtopic.php?f=2&t=46
      - Dominik Fischer / dom_fischer(at)web(dot)de
      - Frank Oltmanns / <first name>.<last name>(at)gmail(dot)com
      - Max Horn / max(at)quendi(dot)de
      - Robert ter Vehn / <first name>.<last name>(at)gmail(dot)com
  
      Project home: https://github.com/sui77/rc-switch/
*/

#if !defined(ESP8266_RF433T_H_)
#define ESP8266_RF433T_H_ 1

#  if defined(ENABLE_SENSOR_RF433)

#  if !defined(INTERNAL_RF433_PIN)
#    define INTERNAL_RF433_PIN 16U
#  endif

struct RF433_HighLow_t {
        uint8_t high;
        uint8_t low;
}  __attribute__((packed));

struct RF433_Protocol_t {
        uint16_t pulseLength;
        RF433_HighLow_t syncFactor;
        RF433_HighLow_t zero;
        RF433_HighLow_t one;
        bool invertedSignal;
} __attribute__((packed));

#  if (defined(ESP2866) || defined(ESP32))
static const RF433_Protocol_t PROGMEM proto_[7] = {
#  else
static const RF433_Protocol_t proto_[7] = {
#  endif
    { 350, {  1, 31 }, {  1,  3 }, {  3,  1 }, false },    // protocol 1
    { 650, {  1, 10 }, {  1,  2 }, {  2,  1 }, false },    // protocol 2
    { 100, { 30, 71 }, {  4, 11 }, {  9,  6 }, false },    // protocol 3
    { 380, {  1,  6 }, {  1,  3 }, {  3,  1 }, false },    // protocol 4
    { 500, {  6, 14 }, {  1,  2 }, {  2,  1 }, false },    // protocol 5
    { 450, { 23,  1 }, {  1,  2 }, {  2,  1 }, true  },    // protocol 6 (HT6P20B)
    { 150, {  2, 62 }, {  1,  6 }, {  6,  1 }, false }     // protocol 7 (HS2303-PT, i. e. used in AUKEY Remote)
};

template <const uint8_t TPIN = INTERNAL_RF433_PIN, const uint8_t TREPEAT = 6 /* 10 */>
class ESP8266_RF433T {
  private:
    bool istransmit_;
    RF433_Protocol_t protocol_;

  public:
    ESP8266_RF433T() {
        pinMode(TPIN, OUTPUT);
        protocol(1);
        istransmit_ = false;
    }
    void send(unsigned long code, uint16_t length) {
        if (!istransmit_)
            return;

        for (uint8_t n = 0U; n <= TREPEAT; ++n) {
            for (int8_t i = (length - 1); i >= 0; i--) {
                if (code & (1UL << i))
                    rf433_transmit_(protocol_.one);
                else
                    rf433_transmit_(protocol_.zero);
            }
            rf433_transmit_(protocol_.syncFactor);
        }
        digitalWrite(TPIN, LOW);
    }
    void send(const char* s) {
        if (!istransmit_)
            return;
        
        unsigned long code = 0UL;
        uint16_t length = 0U;
        for (const char* p = s; *p; p++) {
            code <<= 1L;
            if (*p != '0')
                code |= 1L;
            length++;
        }
        send(code, length);
    }
    void sendTriState(const char* s) {
        if (!istransmit_)
            return;
        
        unsigned long code = 0UL;
        uint16_t length = 0U;
        for (const char* p = s; *p; p++) {
            code <<= 2L;
            switch (*p) {
                case 'F': code |= 1L; break;
                case '1': code |= 3L; break;
                default: break;
            }
            length += 2;
        }
        send(code, length);
    }
    void transmit(bool b) {
        istransmit_ = b;
    }
    void pulse(uint16_t pl) {
        protocol_.pulseLength = pl;
    }
    void protocol(RF433_Protocol_t p) {
        protocol_ = p;
    }
    void protocol(uint8_t p) {
        if ((p < 1U) || (p >= __NELE(proto_)))
            p = 1U;
#       if (defined(ESP8266) || defined(ESP32))
        protocol_ = proto_[(p - 1U)];
#       else
        (void) memcpy_P(&protocol_, &proto_[(p - 1U)], sizeof(RF433_Protocol_t));
#       endif
    }
    void protocol(uint8_t p, uint16_t pl) {
        protocol(p);
        pulse(pl);
    }

  private:
    void rf433_transmit_(RF433_HighLow_t & p) {
        uint8_t first  = ((protocol_.invertedSignal) ? LOW : HIGH),
                second = ((protocol_.invertedSignal) ? HIGH : LOW);
        
        digitalWrite(TPIN, first);
        delayMicroseconds(protocol_.pulseLength * p.high);
        digitalWrite(TPIN, second);
        delayMicroseconds(protocol_.pulseLength * p.low);
    }
};
#  endif
#endif
