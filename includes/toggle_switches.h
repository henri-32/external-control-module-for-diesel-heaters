#pragma once 
#include "interfaces.h"

class ToggleSwitch : public IToggleSwitch {
public:
  explicit ToggleSwitch(uint8_t pin);

  void init() override;
  bool changed() override;

private:
  const uint8_t m_pin;
  bool m_current;
  bool m_prev;
  unsigned long m_last_debounce_ms = 0;
  static constexpr unsigned long m_debounce_delay_ms = DebounceConfig::toggleSwitch;
};

