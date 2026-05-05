#include "temperature_sensor.h"
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


