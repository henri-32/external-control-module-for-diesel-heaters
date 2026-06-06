#include "library_adapter.h"
#include <Wire.h>

void LCDAdapter::clear() { liquidCrystal_I2C.clear(); }

void LCDAdapter::noDisplay() { liquidCrystal_I2C.noDisplay(); }

void LCDAdapter::display() { liquidCrystal_I2C.display(); }

void LCDAdapter::noBacklight() { liquidCrystal_I2C.noBacklight(); }

void LCDAdapter::backlight() { liquidCrystal_I2C.backlight(); }

void LCDAdapter::setCursor(uint8_t col, uint8_t row) {
  liquidCrystal_I2C.setCursor(col, row);
}

void LCDAdapter::printstr(const char text[]) {
  liquidCrystal_I2C.printstr(text);
}

void LCDAdapter::init() {
  Wire.begin();
  liquidCrystal_I2C.init();
}

int EncoderAdapter::read() { return libraryObject.read(); }

void TempSensorAdapter::begin() { libraryObject.begin(); }

void TempSensorAdapter::requestTemperatures() {
  libraryObject.requestTemperatures();
}

float TempSensorAdapter::getTempCByIndex(uint8_t index) {return libraryObject.getTempCByIndex(index);}
