#pragma once

#include <Arduino.h>

class Encoder {
public:
  Encoder(uint8_t, uint8_t) {}
  long read() const { return 0L; }
};
