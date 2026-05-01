#include "relaisdriver.h"

RelaisDriver::RelaisDriver(const uint8_t pin) : m_pin(pin) {}

void RelaisDriver::init() {
  pinMode(m_pin, OUTPUT);
  digitalWrite(m_pin, LOW);
}

void RelaisDriver::checkForTurnOn(){};
void RelaisDriver::checkForTurnOff(){};


void RelaisDriver::update(const ControllerOutputIntent::RelaisCommand& intent) {
  if (m_relais_state == RelaisState::OFF) {
    /*muss hier als positiv Bedinung behandelt werden, damit der zweite Teil
     der Funktion (Überprüfung des Ablaufs) unabhängig vom Zustand erfolgt*/
    if (intent == ControllerOutputIntent::RelaisCommand::None)
      return;
    applyPulseLengthFromIntent(intent);
    m_pulse_start_ms = millis();
    activate();
    m_relais_state = RelaisState::ON;
  } else if (m_relais_state == RelaisState::ON) {
    if (millis() - m_pulse_start_ms < m_pulse_ms)
      return;
    deactivate();
    m_relais_state = RelaisState::OFF;
  }
}

void RelaisDriver::applyPulseLengthFromIntent(
    ControllerOutputIntent::RelaisCommand intent_copy) {
  if (intent_copy == ControllerOutputIntent::RelaisCommand::Long)
    m_pulse_ms = 1500;
  else if (intent_copy == ControllerOutputIntent::RelaisCommand::Short)
    m_pulse_ms = 200;
  else if (intent_copy == ControllerOutputIntent::RelaisCommand::None)
    m_pulse_ms = 0;
}
