#include "test_devices.h"

void TestOutputDevices::update() {
  //{{{
  // Simuliert das Verhalten der echten Klasse.
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
  // Diese Testklasse hält lastCommand als Zustand. Die echte Relais-Klasse
  // tut das nicht. Das dient nur dazu, die Aufrufe an das Relais
  // nachzuverfolgen, und entspricht logisch nicht dem echten Treiber.
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
