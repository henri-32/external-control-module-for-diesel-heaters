#include "pushbuttons.h"

#ifdef TEST_BUILD
#include "ArduinoStubs.h"
#else
#include "Arduino.h"
#endif

PushButton::PushButton(uint8_t pin) : m_pin(pin) {}

void PushButton::init() {
  pinMode(m_pin, INPUT_PULLUP);
  m_stable = digitalRead(m_pin);
  m_last_read = m_stable;
}

bool PushButton::pressed() const { return m_stable == LOW; };
bool PushButton::released() {
  bool reading = digitalRead(m_pin);

  if (reading != m_last_read) {
    m_last_debounce_ms = millis();
    m_last_read = reading;
  }

  if (millis() - m_last_debounce_ms < kDebounceDelayMs)
    return false;

  if (m_stable != reading) {
    m_stable = reading;
    return m_stable == HIGH; // nur bei stabilem LOW-> HIGH
  }

  return false;
}
