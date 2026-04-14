#include "devicegroups.h"

InputDevices::InputDevices(ControllerInputData &id) : m_devices_data(id) {}

void InputDevices::init() {
  m_powerSwitch.init();
  m_modeSwitch.init();
  m_displayButton.init();
  m_myEncoder.init();
  m_tempSensor.init();
}

void InputDevices::updateInputData() {
  m_devices_data.alternatorPressed = m_displayButton.isDown();
  m_devices_data.powerSwitchChanged = m_powerSwitch.changed();
  m_devices_data.modeSwitchChanged = m_modeSwitch.changed();
  m_devices_data.displayButtonReleased = m_displayButton.released();
  m_devices_data.encoder_val = m_myEncoder.readSteps();
  m_devices_data.sensor_tempC = m_tempSensor.pollTemp();
}

OutputDevices::OutputDevices(ControllerOutputIntent &oi)
    : m_devices_intent(oi), m_lcdDisplay(m_devices_intent.displayContent,
                                         m_devices_intent.lcd_stateIntent) {}

void OutputDevices::init() {
  m_lcdDisplay.init();
  m_relais.init();
}

void OutputDevices::update() {
  m_relais.update(m_devices_intent.consumeRelaisRequest());
  m_lcdDisplay.update();
}
