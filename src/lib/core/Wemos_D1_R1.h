#if !defined(__WEMOS_D1_R1_H)
#define __WEMOS_D1_R1_H 1

/* Wemos D1 R1 Pinout
       name | gpio | desc
*/
#  define WD0 3   //
#  define WD1 1   //
#  define WD2 16  //
#  define WD3 5   // SCL (I2C) -- dup
#  define WD4 4   // SDA (I2C) -- dup
#  define WD5 14  // 
#  define WD6 12  // 
#  define WD7 13  // 
#  define WD8 0   //
#  define WD9 2   // CE (SPI)
#  define WD10 15 // CSN (SPI)
#  define WD11 13 // MOSI (SPI)
#  define WD12 12 // MISO (SPI)
#  define WD13 14 // SCK (SPI)
#  define WD14 4  // SDA (I2C) -- dup
#  define WD15 5  // SCL (I2C) -- dup
#  define WA0 A0  // Lighting resistor

#endif
