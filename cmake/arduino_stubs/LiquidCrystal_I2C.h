#pragma once

#include <Arduino.h>

class LiquidCrystal_I2C {
public:
  LiquidCrystal_I2C(uint8_t, uint8_t, uint8_t) {}

  void init() {}
  void backlight() {}
  void noBacklight() {}
  void clear() {}
  void display() {}
  void noDisplay() {}
  void setCursor(uint8_t, uint8_t) {}
  void print(const char *) {}
};
