#include "test_devices.h"

void TestOutputDevices::update() {
  //{{{
  // simulates the behavior of the real Class,
  m_relais.update(intent.relaisCommand);
  intent.relaisCommand = OutputDevicesIntent::RelaisCommand::None;
}
//}}}

void TestRelais::update(OutputDevicesIntent::RelaisCommand intent) {
  //{{{
  lastCommand = intent;
}
//}}}

OutputDevicesIntent::RelaisCommand TestRelais::receivedCommand() {
  //{{{
  // This TestClass holds the lastCommand as a state. The real relais
  // class does not. This is only intended to keep track of the calls
  // to the relais and does not logically hold up to the real driver.
  OutputDevicesIntent::RelaisCommand lc = lastCommand;
  lastCommand = OutputDevicesIntent::RelaisCommand::None;
  return lc;
};
//}}}
void TestRelais::reset() {
//{{{
 lastCommand = OutputDevicesIntent::RelaisCommand::None;  
}
//}}}

void TestDisplayHardware::init() { m_init_called = true; }

int TestEncoderHardware::read() { return position; }

void TestTemperatureSensorHardware::requestTemperatures() {
  ///{{{
  requestTemperaturesCount++;
}
//}}}

float TestTemperatureSensorHardware::getTempCByIndex(uint8_t index) {
  //{{{
  (void)index;
  getTempCByIndexCount++;
  return m_tempReturn;
}
//}}}

void TestTemperatureSensorHardware::setTempReturn(float temp) {
  //{{{
  m_tempReturn = temp;
}
//}}}

void TestTemperatureSensorHardware::resetTestCounter() {
  //{{{
  requestTemperaturesCount = 0;
  getTempCByIndexCount = 0;
};
//}}}

void TestToggleSwitch::setNextChangedReturn(bool nr) {nextReturn = nr;}

void TestPushButton::setNextPressedReturn(bool nr) { nextPressed = nr; }

void TestPushButton::setNextReleasedReturn(bool nr) { nextReleased = nr; }
