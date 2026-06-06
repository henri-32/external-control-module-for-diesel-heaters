#include "toggle_switches.h"

#ifdef TEST_BUILD
#include "ArduinoStubs.h"
#else 
#include "Arduino.h"
#endif 

ToggleSwitch::ToggleSwitch(uint8_t pin) : m_pin(pin){};

void ToggleSwitch::init() {
  pinMode(m_pin, INPUT_PULLUP);
  m_current = digitalRead(m_pin);
  m_prev = m_current;
};

bool ToggleSwitch::changed() {
  m_current = digitalRead(m_pin);
  if (m_current == m_prev)
    return false;
  if (millis() - m_last_debounce_ms < kDebounceDelayMs)
    return false;

  m_prev = m_current;
  m_last_debounce_ms = millis();
  return true;
};
