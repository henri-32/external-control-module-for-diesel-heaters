#include "test_devices.h"

void TestDisplay::init() { m_init_called = true; }

int TestEncoderHardware::read() { return position; }

float TestTemperatureSensorHardware::getTempCByIndex(uint8_t index) {
  (void)index;
  return m_tempReturn;
}
void TestTemperatureSensorHardware::setTempReturn(float temp) {
  m_tempReturn = temp;
}
