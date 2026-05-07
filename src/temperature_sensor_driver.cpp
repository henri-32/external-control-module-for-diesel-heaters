#include "temperature_sensor_driver.h"

#ifdef TEST_BUILD 
#include "ArduinoStubs.h"
#else 
#include "Arduino.h"
#endif 

TemperatureSensorDriver::TemperatureSensorDriver(
    ITempSensorHardware &sensorHardware)
    : m_sensorHardware(sensorHardware) {
}

void TemperatureSensorDriver::init() {
  m_sensorHardware.begin();
  m_sensorHardware.requestTemperatures();
  m_temp_c = m_sensorHardware.getTempCByIndex(0);
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
  m_sensorHardware.requestTemperatures();
  m_last_temp_request = millis();
  m_tempRequestPending = true;
}

void TemperatureSensorDriver::measureTemperature() {
  if (!m_tempRequestPending)
    return;
  if (millis() - m_last_temp_request < m_conversion_time_ms)
    return;
  m_temp_c = m_sensorHardware.getTempCByIndex(0);
  m_tempRequestPending = false;
};

