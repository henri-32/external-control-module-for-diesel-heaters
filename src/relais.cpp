#include "relais.h"
#include "config.h"

Relais::Relais(const uint8_t pin) : m_pin(pin) {}

void Relais::init() {
  pinMode(m_pin, OUTPUT);
  digitalWrite(m_pin, LOW);
}

void Relais::update(OutputDevicesIntent::RelaisCommand command) {
  turnOn(command);
  turnOff();
}

void Relais::turnOn(const OutputDevicesIntent::RelaisCommand &command) {
  if (m_relais_state != RelaisState::OFF ||
      command == OutputDevicesIntent::RelaisCommand::None) {
    return;
  }
  applyPulseLengthFromIntent(command);
  activate();
  m_pulse_start_ms = millis();
  m_relais_state = RelaisState::ON;
};

void Relais::turnOff() {
  if (m_relais_state != RelaisState::ON) {
    return;
  }
  if (millis() - m_pulse_start_ms < m_pulse_ms) {
    return;
  }
  deactivate();
  m_relais_state = RelaisState::OFF;
}


void Relais::applyPulseLengthFromIntent(
    const OutputDevicesIntent::RelaisCommand &command) {
  using cmd = OutputDevicesIntent::RelaisCommand;

  switch (command) {
  case cmd::None:
    return;
  case cmd::Long:
    m_pulse_ms = Config::RelaisLongPulse_ms;
    break;

  case cmd::Short:
    m_pulse_ms = Config::RelaisShortPulse_ms;
    break;
  }
}
