#include "test_devices.h"

void TestOutputDevices::update() {
  //simulates the behavior of the real Class, 
  m_relais.update(intent.relaisCommand);
  intent.relaisCommand = OutputDevicesIntent::RelaisCommand::None;
}

void TestRelais::update(OutputDevicesIntent::RelaisCommand intent) {
  lastCommand = intent;
}

OutputDevicesIntent::RelaisCommand
TestRelais::returnLastRecievedCommand() {
//{{{
//This TestClass holds the lastCommand as a state. The real relais
//class does not. This is only intended to keep track of the calls
//to the relais and does not logically hold up to the real driver.
  OutputDevicesIntent::RelaisCommand lc = lastCommand; 
  lastCommand = OutputDevicesIntent::RelaisCommand::None; 
  return lc;
};
//}}}

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
