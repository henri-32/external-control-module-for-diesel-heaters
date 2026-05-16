#include "test_devices.h"

void TestDisplay::init() { m_init_called = true; }

int TestEncoderHardware::read() { return position; }

void TestTemperatureSensorHardware::requestTemperatures() {
  requestTemperaturesCount++;
}

float TestTemperatureSensorHardware::getTempCByIndex(uint8_t index) {
  (void)index;
  getTempCByIndexCount++;
  return m_tempReturn;
}
void TestTemperatureSensorHardware::setTempReturn(float temp) {
  m_tempReturn = temp;
}

void TestTemperatureSensorHardware::resetTestCounter() {
  requestTemperaturesCount = 0;
  getTempCByIndexCount = 0;
};
