#pragma once

#include <Arduino.h>
#include <OneWire.h>

class DallasTemperature {
public:
  explicit DallasTemperature(OneWire *) {}

  void begin() {}
  void requestTemperatures() {}
  float getTempCByIndex(uint8_t) const { return 20.0f; }
};
