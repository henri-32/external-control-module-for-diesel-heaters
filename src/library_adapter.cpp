#include "library_adapter.h"
#include <Wire.h>

void LCDAdapter::clear() { liquidCristal_I2C.clear(); }

void LCDAdapter::noDisplay() { liquidCristal_I2C.noDisplay(); }

void LCDAdapter::display() { liquidCristal_I2C.display(); }

void LCDAdapter::noBacklight() { liquidCristal_I2C.noBacklight(); }

void LCDAdapter::backlight() { liquidCristal_I2C.backlight(); }

void LCDAdapter::setCursor(uint8_t col, uint8_t row) {
  liquidCristal_I2C.setCursor(col, row);
}

void LCDAdapter::printstr(const char text[]) {
  liquidCristal_I2C.printstr(text);
}

void LCDAdapter::init() {
  Wire.begin();
  liquidCristal_I2C.init();
}

int EncoderAdapter::read() {return m_encoder.read();} 
