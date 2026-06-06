#pragma once

#include "interfaces.h"

class Relais : public IRelais {
public:
  explicit Relais(const uint8_t pin);

  void init() override;
  void update(OutputDevicesIntent::RelaisCommand intent) override;

private:
  void turnOn(const OutputDevicesIntent::RelaisCommand &intent);
  void turnOff();
  void applyPulseLengthFromIntent(
      const OutputDevicesIntent::RelaisCommand &command);
  void activate(); 
  void deactivate() ;

  const uint8_t m_pin;
  enum class RelaisState { On, Off };
  RelaisState m_relais_state = RelaisState::Off;
  uint16_t m_pulse_ms = 0;
  unsigned long m_pulse_start_ms = 0;
};
