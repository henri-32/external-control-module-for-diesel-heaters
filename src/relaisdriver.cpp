#include "relaisdriver.h"

RelaisDriver::RelaisDriver(const uint8_t pin) : m_pin(pin) {}

void RelaisDriver::init() {
  pinMode(m_pin, OUTPUT);
  digitalWrite(m_pin, LOW);
}

void RelaisDriver::turnOn(const ControllerOutputIntent::RelaisCommand &intent) {
  if (m_relais_state != RelaisState::OFF ||
      intent == ControllerOutputIntent::RelaisCommand::None) {
    return;
  }
  applyPulseLengthFromIntent(intent);
  activate();
  m_pulse_start_ms = millis();
  m_relais_state = RelaisState::ON;
};

void RelaisDriver::turnOff() {
  if (m_relais_state != RelaisState::ON) {
    return;
  }
  if (millis() - m_pulse_start_ms < m_pulse_ms) {
    return;
  }
  deactivate();
  m_relais_state = RelaisState::OFF;
};

void RelaisDriver::update(const ControllerOutputIntent::RelaisCommand &intent) {
  turnOn(intent);
  turnOff();
}

void RelaisDriver::applyPulseLengthFromIntent(
    const ControllerOutputIntent::RelaisCommand &intent) {
  using cmd = ControllerOutputIntent::RelaisCommand;

  switch (intent) {
  case cmd::None:
    return;
  case cmd::Long:
    m_pulse_ms = 1500;
    break;

  case cmd::Short:
    m_pulse_ms = 200;
    break;
  }
}
