#pragma once
#include "config.h"
#include "displaydriver.h"
#include "hardwaredrivers.h"
#include "types.h"

class InputDevices {
public:
  InputDevices(ControllerInputData &id);

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

class OutputDevices {
public:
  OutputDevices(ControllerOutputIntent &oi);

  void init();
  void update();

  RelaisDriver m_relais{my_pin_config::relais};
  ControllerOutputIntent &m_devices_intent;
  DisplayDriver m_lcdDisplay; // Keine PIN Zuweisung nötig, da Arduino I2C Bus
                              // automatisch erkennt.
};
