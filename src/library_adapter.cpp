#include "library_adapter.h"


//Implementierungen (Weitergabe von API Calls) des LCDAdapter 
//{{{
void LCDAdapter::begin(uint8_t cols, uint8_t rows, uint8_t charsize) {
  liquidCristal_I2C.begin(cols, rows, charsize);
}

void LCDAdapter::clear() { liquidCristal_I2C.clear(); }

void LCDAdapter::homevirtual() { liquidCristal_I2C.home(); }

void LCDAdapter::noDisplay() { liquidCristal_I2C.noDisplay(); }

void LCDAdapter::display() { liquidCristal_I2C.display(); }

void LCDAdapter::noBlink() { liquidCristal_I2C.noBlink(); }

void LCDAdapter::blink() { liquidCristal_I2C.blink(); }

void LCDAdapter::noCursor() { liquidCristal_I2C.noCursor(); }

void LCDAdapter::cursor() { liquidCristal_I2C.cursor(); }

void LCDAdapter::scrollDisplayLeft() { liquidCristal_I2C.scrollDisplayLeft(); }

void LCDAdapter::scrollDisplayRight() {
  liquidCristal_I2C.scrollDisplayRight();
}

void LCDAdapter::leftToRight() { liquidCristal_I2C.leftToRight(); }

void LCDAdapter::rightToLeft() { liquidCristal_I2C.rightToLeft(); }

void LCDAdapter::noBacklight() { liquidCristal_I2C.noBacklight(); }

void LCDAdapter::backlight() { liquidCristal_I2C.backlight(); }

void LCDAdapter::autoscroll() { liquidCristal_I2C.autoscroll(); }

void LCDAdapter::noAutoscroll() { liquidCristal_I2C.noAutoscroll(); }

void LCDAdapter::createChar(uint8_t location, uint8_t charmap[]) {
  liquidCristal_I2C.createChar(location, charmap);
}

void LCDAdapter::createChar(uint8_t location, const char *charmap) {
  liquidCristal_I2C.createChar(location, charmap);
}

void LCDAdapter::setCursor(uint8_t col, uint8_t row) {
  liquidCristal_I2C.setCursor(col, row);
}

void LCDAdapter::blink_on() { liquidCristal_I2C.blink_on(); }

void LCDAdapter::blink_off() { liquidCristal_I2C.blink_off(); }

void LCDAdapter::cursor_on() { liquidCristal_I2C.cursor_on(); }

void LCDAdapter::cursor_off() { liquidCristal_I2C.cursor_off(); }

void LCDAdapter::setBacklight(uint8_t new_val) {
  liquidCristal_I2C.setBacklight(new_val);
}

void LCDAdapter::load_custom_character(uint8_t char_num, uint8_t *rows) {
  liquidCristal_I2C.load_custom_character(char_num, rows);
}

void LCDAdapter::printstr(const char text[]) {
  liquidCristal_I2C.printstr(text);
}

void LCDAdapter::init() { liquidCristal_I2C.init(); }

void LCDAdapter::update() {}
//}}}
