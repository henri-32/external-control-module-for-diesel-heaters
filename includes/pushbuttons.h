#pragma once
#include "interfaces.h"

#ifdef TEST_BUILD
#include "ArduinoStubs.h"
#else
#include "Arduino.h"
#endif

class PushButton : public IPushButton {
public:
  explicit PushButton(uint8_t pin);

  void init() override;
  bool isDown() const override { return m_stable == LOW; }
  bool released() override;

private:
  const uint8_t m_pin;
  bool m_stable;
  bool m_last_read;
  unsigned long m_last_debounce_ms = 0;
  static constexpr unsigned long m_debounce_delay_ms = 50;
};
