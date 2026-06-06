#pragma once
#include "interfaces.h"

class PushButton : public IPushButton {
public:
  explicit PushButton(uint8_t pin);

  void init() override;
  bool pressed() const override; 
  bool released() override;

private:
  const uint8_t m_pin;
  bool m_stable;
  bool m_last_read;
  unsigned long m_last_debounce_ms = 0;
  static constexpr unsigned long kDebounceDelayMs = DebounceConfig::kPushButtonMs;
};
