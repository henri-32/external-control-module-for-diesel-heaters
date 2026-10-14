#include "test_devices.h"

void TestRuntimeConfig::load() {
  // Dieselbe Iplementierung wie im Produktionscode, nur ohne
  // EEPROM read, da die Tests die Werte über den Stub setzen.
  if (!std::isfinite(data.default_tempC.value)) {
    data.default_tempC.value = Config::kDefaultTempC;
  }
};
float TestRuntimeConfig::get_default_tempC() const {
  return data.default_tempC.value;
}

void TestRuntimeConfig::set_default_tempC(float value) {
  data.default_tempC.value = value;
}

void TestRuntimeConfig::set_relais_pulse_short_ms(uint8_t value) {
  data.RelaisPulseShortMs.value = value;
}

uint8_t TestRuntimeConfig::get_relais_pulse_short_ms() const {
  return data.RelaisPulseShortMs.value;
}

void TestOutputDevices::update() {
  //{{{
  // Simuliert das Verhalten der echten Klasse.
  m_relais.update(intent.relaisCommand);
  intent.relaisCommand = OutputDevicesIntent::RelaisCommand::None;
  m_display.update(intent.displayContent); 
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

void TestToggleSwitch::setNextChangedReturn(bool nr) { nextReturn = nr; }

void TestPushButton::setNextPressedReturn(bool nr) { nextPressed = nr; }

void TestPushButton::setNextReleasedReturn(bool nr) { nextReleased = nr; }
