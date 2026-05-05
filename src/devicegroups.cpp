#include "devicegroups.h"

RealInputDevices::RealInputDevices(ControllerInputData &id, ToggleSwitch &ps,
                                   ToggleSwitch &ms, PushButton &pb,
                                   IEncoder &encoder)
    : m_devices_data(id), m_powerSwitch(ps), m_modeSwitch(ms), m_displayButton(pb),
      m_myEncoder{encoder} {}

void RealInputDevices::init() {
  m_powerSwitch.init();
  m_modeSwitch.init();
  m_displayButton.init();
  m_myEncoder.init();
  m_tempSensor.init();
}

void RealInputDevices::updateInputData() {
  m_devices_data.alternatorPressed = m_displayButton.isDown();
  m_devices_data.powerSwitchChanged = m_powerSwitch.changed();
  m_devices_data.modeSwitchChanged = m_modeSwitch.changed();
  m_devices_data.alternatorReleased = m_displayButton.released();
  m_devices_data.encoder_val = m_myEncoder.readSteps();
  m_devices_data.sensor_tempC = m_tempSensor.pollTemp();
}

RealOutputDevices::RealOutputDevices(ControllerOutputIntent &oi,
                                     IDisplay &display)
    : m_devices_intent(oi),
      m_lcdDisplay(display, m_devices_intent.displayContent,
                   m_devices_intent.lcd_stateIntent) {}

void RealOutputDevices::init() {
  m_lcdDisplay.init();
  m_relais.init();
}

void RealOutputDevices::update() {
  m_relais.update(m_devices_intent.consumeRelaisRequest());
  m_lcdDisplay.update();
}
