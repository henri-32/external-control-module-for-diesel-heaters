#pragma once

#include "interfaces.h"
#include "types.h"
#include <stdint.h>

#ifdef TEST_BUILD
#include "ArduinoStubs.h"
#else
#include <Arduino.h>
#endif

class Relais : public IRelais {
public:
  explicit Relais(const uint8_t pin);

  void init();
  void update(OutputDevicesIntent::RelaisCommand intent) override;

private:
  void turnOn(const OutputDevicesIntent::RelaisCommand &intent);
  void turnOff();
  void applyPulseLengthFromIntent(
      const OutputDevicesIntent::RelaisCommand &command);
  void activate() { digitalWrite(m_pin, HIGH); }
  void deactivate() { digitalWrite(m_pin, LOW); };

  const uint8_t m_pin;
  enum class RelaisState { ON, OFF };
  RelaisState m_relais_state = RelaisState::OFF;
  uint16_t m_pulse_ms = 0;
  unsigned long m_pulse_start_ms = 0;
};
