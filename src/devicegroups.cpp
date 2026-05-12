#include "devicegroups.h"

InputDevices::InputDevices(InputDevicesDataSet &inputData, IToggleSwitch &ps,
                           IToggleSwitch &ms, IPushButton &pb,
                           IEncoderDriver &encoderDriver,
                           ITempSensorDriver &tempSensorDriver)
    : IInputDevices(inputData), m_powerSwitch(ps), m_modeSwitch(ms),
      m_displayButton(pb), m_encoderDriver(encoderDriver),
      m_tempSensorDriver(tempSensorDriver) {}

void InputDevices::init() {
  m_powerSwitch.init();
  m_modeSwitch.init();
  m_displayButton.init();
  m_encoderDriver.init();
  m_tempSensorDriver.init();
}

void InputDevices::update() {
  data.alternator.pressed = m_displayButton.isDown();
  data.switchAction.power = m_powerSwitch.changed();
  data.switchAction.mode = m_modeSwitch.changed();
  data.alternator.released = m_displayButton.released();
  data.encoder_val = m_encoderDriver.readSteps();
  data.sensor_tempC = m_tempSensorDriver.pollTemp();
}

OutputDevices::OutputDevices(OutputDevicesIntent &outputIntent,
                             IDisplayDriver &displayDriver, IRelais &relais)
    :IOutputDevices(outputIntent), m_relais(relais),
      m_displayDriver(displayDriver) {}

void OutputDevices::init() {
  m_displayDriver.init();
  m_relais.init();
}

void OutputDevices::update() {
  m_relais.update(intent.relaisCommand);
  intent.relaisCommand = OutputDevicesIntent::RelaisCommand::None;
  m_displayDriver.update();
}
