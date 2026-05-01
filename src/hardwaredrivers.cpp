#include "hardwaredrivers.h"

ToggleSwitchDriver::ToggleSwitchDriver(uint8_t pin) : m_pin(pin){};

void ToggleSwitchDriver::init() {
  pinMode(m_pin, INPUT_PULLUP);
  m_current = digitalRead(m_pin);
  m_prev = m_current;
};

bool ToggleSwitchDriver::changed() {
  m_current = digitalRead(m_pin);
  if (m_current == m_prev)
    return false;
  if (millis() - m_last_debounce_ms < m_debounce_delay_ms)
    return false;

  m_prev = m_current;
  m_last_debounce_ms = millis();
  return true;
};

PushButtonDriver::PushButtonDriver(uint8_t pin) : m_pin(pin) {}

void PushButtonDriver::init() {
  pinMode(m_pin, INPUT_PULLUP);
  m_stable = digitalRead(m_pin);
  m_last_read = m_stable;
}

bool PushButtonDriver::released() {
  bool reading = digitalRead(m_pin);

  if (reading != m_last_read) {
    m_last_debounce_ms = millis();
    m_last_read = reading;
  }

  if (millis() - m_last_debounce_ms < m_debounce_delay_ms)
    return false;

  if (m_stable != reading) {
    m_stable = reading;
    return m_stable == HIGH; // nur bei stabilem LOW-> HIGH
  }

  return false;
}

MyEncoderDriver::MyEncoderDriver(uint8_t pin1, uint8_t pin2)
    : m_encoder(pin1, pin2){};

int MyEncoderDriver::readSteps() {
  poll();
  return translateStepsToInput();
}

void MyEncoderDriver::poll() {
  m_current = m_encoder.read();
  long diff = m_current - m_prev;
  if (diff == 0)
    return;
  m_delta += diff;
  m_last_change_ms = millis();
  m_prev = m_current;
}

int MyEncoderDriver::translateStepsToInput() {
  if (millis() - m_last_change_ms < 100)
    return 0;
  int steps = m_delta / 4; // Wird auf ganze Schritte runtergebrochen und rest
                           // vernichtet bei Ganzzahldivision
  m_delta -= steps * 4;    // Das Delta wird um die Schritte die ganz gemacht
                           // wurden wieder reduziert (in Rohwerten) so bleibt
                           // der Rest für den nächsten Aufruf erhalten
  return steps;
};

TemperatureSensorDriver::TemperatureSensorDriver(uint8_t pin)
    : m_one_wire(pin), m_sensors(&m_one_wire) {
} // Verknüpfung von DallasTemperature & OneWire

void TemperatureSensorDriver::init() {
  m_sensors.begin();
  m_sensors.requestTemperatures();
  m_temp_c = m_sensors.getTempCByIndex(0);
}

float TemperatureSensorDriver::pollTemp() {
  startTemperatureRequest();
  measureTemperature();
  return m_temp_c;
}

void TemperatureSensorDriver::startTemperatureRequest() {
  if (millis() - m_last_temp_request < m_request_intervall_ms)
    return;
  if (m_tempRequestPending)
    return;
  m_sensors.requestTemperatures();
  m_last_temp_request = millis();
  m_tempRequestPending = true;
}

void TemperatureSensorDriver::measureTemperature() {
  if (!m_tempRequestPending)
    return;
  if (millis() - m_last_temp_request < m_conversion_time_ms)
    return;
  m_temp_c = m_sensors.getTempCByIndex(0);
  m_tempRequestPending = false;
};


