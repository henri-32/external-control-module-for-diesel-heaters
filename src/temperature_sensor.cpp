#include "temperature_sensor.h"

#ifdef TEST_BUILD 
#include "ArduinoStubs.h"
#else 
#include "Arduino.h"
#endif 

TemperatureSensor::TemperatureSensor(ITempSensor& sensor)
    :m_sensors (sensor) {
}

void TemperatureSensor::init() {
  m_sensors.begin();
  m_sensors.requestTemperatures();
  m_temp_c = m_sensors.getTempCByIndex(0);
}

float TemperatureSensor::pollTemp() {
  startTemperatureRequest();
  measureTemperature();
  return m_temp_c;
}

void TemperatureSensor::startTemperatureRequest() {
  if (millis() - m_last_temp_request < m_request_intervall_ms)
    return;
  if (m_tempRequestPending)
    return;
  m_sensors.requestTemperatures();
  m_last_temp_request = millis();
  m_tempRequestPending = true;
}

void TemperatureSensor::measureTemperature() {
  if (!m_tempRequestPending)
    return;
  if (millis() - m_last_temp_request < m_conversion_time_ms)
    return;
  m_temp_c = m_sensors.getTempCByIndex(0);
  m_tempRequestPending = false;
};


