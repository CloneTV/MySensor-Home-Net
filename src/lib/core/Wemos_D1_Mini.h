#if !defined(__WEMOS_D1_MINI_H)
#define __WEMOS_D1_MINI_H 1

/* Wemos D1 Mini Pinout
       name | gpio | desc
*/
#  define WD0 16  // Transmitter 433
#  define WD1 5   // SCL (I2C)
#  define WD2 4   // SDA (I2C)
#  define WD3 0   // -- (FLASH)
#  define WD4 2   // CE (SPI)
#  define WD5 14  // SCK (SPI)
#  define WD6 12  // MISO (SPI)
#  define WD7 13  // MOSI (SPI)
#  define WD8 15  // CSN (SPI)
#  define WTX 1   // -- (TX Serial)
#  define WRX 3   // -- (RX Serial)
#  define WA0 A0  // Lighting resistor

static const uint8_t scl_PIN = WD1;
static const uint8_t sda_PIN = WD2;

#endif
