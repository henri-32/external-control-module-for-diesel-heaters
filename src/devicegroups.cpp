#include "devicegroups.h"

InputDevices::InputDevices(ControllerInputData &inputData, IToggleSwitch &ps,
                           IToggleSwitch &ms, IPushButton &pb,
                           IEncoderDriver &encoderDriver,
                           ITempSensorDriver &tempSensorDriver)
    : m_inputData(inputData), m_powerSwitch(ps), m_modeSwitch(ms),
      m_displayButton(pb), m_encoderDriver(encoderDriver),
      m_tempSensorDriver(tempSensorDriver) {}

void InputDevices::init() {
  m_powerSwitch.init();
  m_modeSwitch.init();
  m_displayButton.init();
  m_encoderDriver.init();
  m_tempSensorDriver.init();
}

void InputDevices::updateInputData() {
  m_inputData.alternatorPressed = m_displayButton.isDown();
  m_inputData.powerSwitchChanged = m_powerSwitch.changed();
  m_inputData.modeSwitchChanged = m_modeSwitch.changed();
  m_inputData.alternatorReleased = m_displayButton.released();
  m_inputData.encoder_val = m_encoderDriver.readSteps();
  m_inputData.sensor_tempC = m_tempSensorDriver.pollTemp();
}

OutputDevices::OutputDevices(ControllerOutputIntent &outputIntent,
                             IDisplayDriver &displayDriver, IRelais &relais)
    : m_outputIntent(outputIntent), m_relais(relais),
      m_displayDriver(displayDriver) {}

void OutputDevices::init() {
  m_displayDriver.init();
  m_relais.init();
}

void OutputDevices::update() {
  m_relais.update(m_outputIntent.consumeRelaisRequest());
  m_displayDriver.update();
}
