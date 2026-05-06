#include "devicegroups.h"

InputDevices::InputDevices(ControllerInputData &id, IToggleSwitch &ps,
                                   IToggleSwitch &ms, IPushButton &pb,
                                   IEncoder &encoder, ITempSensor & sensor)
    : m_devices_data(id), m_powerSwitch(ps), m_modeSwitch(ms), m_displayButton(pb),
      m_myEncoder{encoder}, m_tempSensor{sensor} {}

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
  m_devices_data.alternatorReleased = m_displayButton.released();
  m_devices_data.encoder_val = m_myEncoder.readSteps();
  m_devices_data.sensor_tempC = m_tempSensor.pollTemp();
}

OutputDevices::OutputDevices(ControllerOutputIntent &oi,
                                     IDisplay &display, IRelais& relais)
    : m_devices_intent(oi),
      m_relais(relais),
      m_lcdDisplay(display, m_devices_intent.displayContent,
                   m_devices_intent.lcd_stateIntent) {}

void OutputDevices::init() {
  m_lcdDisplay.init();
  m_relais.init();
}

void OutputDevices::update() {
  m_relais.update(m_devices_intent.consumeRelaisRequest());
  m_lcdDisplay.update();
}
