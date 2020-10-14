#if !defined(ESP2866_LCD1602_I2C_H)
#define ESP2866_LCD1602_I2C_H

//#define WIRE_INIT_ENABLE 1

#  if (defined(ESP2866) || defined(ESP32))
#    include <ESP.h>
#  elif (defined(ARDUINO) && (ARDUINO >= 100))
#    include "Arduino.h"
#  elif defined(ARDUINO)
#    include "WProgram.h"
#  endif
#  include <inttypes.h>
#  include <Wire.h>

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

	/**
	 * Template construct
	 *
	 * @param LCD_ADDR	I2C slave address of the LCD display. Most likely printed on the
	 *			LCD circuit board, or look in the supplied LCD documentation.
	 *			default: 0x27
	 * @param LCD_COLUMS	Number of columns your LCD display has.
	 * @param LCD_ROWS	Number of rows your LCD display has.
	 * @param I2CP_SDA	I2C SDA pin.
	 * @param I2CP_SCL	I2C SCL pin.
	 **/

template <uint8_t const LCD_ADDR = 0x27,
	  uint8_t const LCD_COLUMS = 16U, uint8_t const LCD_ROWS = 2U,
	  uint8_t const I2CP_SDA = 4U,    uint8_t const I2CP_SCL = 5U>
class ESP2866_LCD1602_I2C : public Print {
	private:
  bool isSuccessfully;
	uint8_t displayfunction__;
	uint8_t displaycontrol__;
	uint8_t displaymode__;
	uint8_t backlightval__;

  public:
	ESP2866_LCD1602_I2C() {
		backlightval__ = LCD_BACKLIGHT;
    isSuccessfully = false;
	}
	~ESP2866_LCD1602_I2C() {
		clear();
		display(false);
	}
  operator bool() const {
    return isSuccessfully;
  }
	bool begin(uint8_t charsize = LCD_5x8DOTS) {
#		if (defined(WIRE_INIT_ENABLE) && (WIRE_INIT_ENABLE > 0))
		Wire.begin(I2CP_SDA, I2CP_SCL);
		delay(150);
#		endif
    Wire.beginTransmission(LCD_ADDR);
    if (Wire.endTransmission() != 0)
      return isSuccessfully;

		displayfunction__ = LCD_4BITMODE | LCD_1LINE | charsize;

		if (LCD_ROWS > 1U) {
			displayfunction__ |= LCD_2LINE;
		}
		lcd_expanderWrite_(backlightval__); delay(1000);
		lcd_write4bits_(0x03 << 4); delayMicroseconds(4500);
		lcd_write4bits_(0x03 << 4); delayMicroseconds(4500);
		lcd_write4bits_(0x03 << 4); delayMicroseconds(150);
		lcd_write4bits_(0x02 << 4); delayMicroseconds(150);
		lcd_command_(LCD_FUNCTIONSET | displayfunction__);
		displaycontrol__ = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
		display();
		clear();
		displaymode__ = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
		lcd_command_(LCD_ENTRYMODESET | displaymode__);
		home();
    isSuccessfully = true;
    return isSuccessfully;
	}
	void clear(bool b = true) {
		lcd_command_(LCD_CLEARDISPLAY);
		if (b)
			delayMicroseconds(2000);
	}
	void home(bool b = true) {
		lcd_command_(LCD_RETURNHOME);
		if (b)
			delayMicroseconds(2000);
	}
	void display(bool b = true) {
		if (b)
			displaycontrol__ |= LCD_DISPLAYON;
		else
			displaycontrol__ &= ~LCD_DISPLAYON;
		lcd_command_(LCD_DISPLAYCONTROL | displaycontrol__);
	}
	void cursor(bool b = true) {
		if (b)
			displaycontrol__ |= LCD_CURSORON;
		else
			displaycontrol__ &= ~LCD_CURSORON;
		lcd_command_(LCD_DISPLAYCONTROL | displaycontrol__);
	}
	void cursor(uint8_t col, uint8_t row) {
		int16_t offsets[] = { 0x00, 0x40, 0x14, 0x54 };
		if (row > LCD_ROWS) {
			row = (LCD_ROWS - 1);
		}
		lcd_command_(LCD_SETDDRAMADDR | (col + offsets[row]));
	}
	void blink(bool b = true) {
		if (b)
			displaycontrol__ |= LCD_BLINKON;
		else
			displaycontrol__ &= ~LCD_BLINKON;
		lcd_command_(LCD_DISPLAYCONTROL | displaycontrol__);
	}
	void leftOrRight(bool b = true) {
		if (b)
			displaymode__ |= LCD_ENTRYLEFT;
		else
			displaymode__ &= ~LCD_ENTRYLEFT;
		lcd_command_(LCD_ENTRYMODESET | displaymode__);
	}
	void autoscroll(bool b = true) {
		if (b)
			displaymode__ |= LCD_ENTRYSHIFTINCREMENT;
		else
			displaymode__ &= ~LCD_ENTRYSHIFTINCREMENT;
		lcd_command_(LCD_ENTRYMODESET | displaymode__);
	}
	void scrollLeft() {
		lcd_command_(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
	}
	void scrollRight() {
		lcd_command_(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
	}
	void backlight(bool b = true) {
		if (b)
			backlightval__ = LCD_BACKLIGHT;
		else
			backlightval__ = LCD_NOBACKLIGHT;
		lcd_expanderWrite_(0);
	}
	void createChar(uint8_t location, uint8_t charmap[]) {
		location &= 0x7;
		lcd_command_(LCD_SETCGRAMADDR | (location << 3));
		for (uint8_t i = 0; i < 8; i++)
			write(charmap[i]);
	}
	// virtual size_t Print::write(uint8_t)
	size_t write(uint8_t val) {
		lcd_send_(val, Rs);
	}

	private:
	void lcd_command_(uint8_t val) {
		lcd_send_(val, 0);
	}
	void lcd_send_(uint8_t val, uint8_t mode) {
		uint8_t highnib = (val & 0xf0);
		uint8_t lownib = ((val << 4) & 0xf0);
		lcd_write4bits_((highnib) | mode);
		lcd_write4bits_((lownib)  | mode);
	}
	void lcd_write4bits_(uint8_t val) {
		lcd_expanderWrite_(val);
		lcd_pulseEnable_(val);
	}
	void lcd_expanderWrite_(uint8_t _data) {                                        
		Wire.beginTransmission(LCD_ADDR);
		Wire.write((int)(_data) | backlightval__);
		Wire.endTransmission();
	}
	void lcd_pulseEnable_(uint8_t _data) {
		lcd_expanderWrite_(_data | En);
		delayMicroseconds(1);
		lcd_expanderWrite_(_data & ~En);
		delayMicroseconds(50);
	}
};

#endif
