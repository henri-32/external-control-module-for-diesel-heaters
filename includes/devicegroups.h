#pragma once
#include "config.h"
#include "displaydriver.h"
#include "hardwaredrivers.h"

#include "types.h"
#include "interfaces.h"

class RealInputDevices : public  IInputDevices {
public:
  RealInputDevices(ControllerInputData &id);

  void init();
  void updateInputData();

  ToggleSwitchDriver m_powerSwitch{my_pin_config::powerSwitch};
  ToggleSwitchDriver m_modeSwitch{my_pin_config::modeSwitch};
  PushButtonDriver m_displayButton{my_pin_config::displayButton};
  MyEncoderDriver m_myEncoder{my_pin_config::myEncoder[0],
                              my_pin_config::myEncoder[1]};
  TemperatureSensorDriver m_tempSensor{my_pin_config::tempSensor};
  ControllerInputData &m_devices_data;
};

class RealOutputDevices : public IOutputDevices {
public:
  RealOutputDevices(ControllerOutputIntent &oi, IDisplay& display);

  void init() override;
  void update() override;

  ControllerOutputIntent &m_devices_intent;
  RelaisDriver m_relais{my_pin_config::relais};
  DisplayDriver m_lcdDisplay;
};
