
#include <ESP.h>
#include "ESP2866_LCD1602_I2C.h"

ESP2866_LCD1602_I2C<> *lcd;

void setup() {
  lcd = new ESP2866_LCD1602_I2C<>();
  lcd->begin();
  lcd->backlight(true);
}

void loop() {
	bool blinking = true;
	lcd->cursor(0, 1);

	while (1) {
		lcd->clear();
		lcd->home();
		
		if (blinking) {
			lcd->print("Cursor blink");
			lcd->blink(blinking);
		} else {
			lcd->print("No cursor blink");
			lcd->blink(blinking);
		}
		blinking = !blinking;
		delay(4000);
	}
}

