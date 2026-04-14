#pragma once
#include "hardwaredrivers.h"
#include "types.h"

class InputDevices {
public:
  InputDevices(ControllerInputData &id);

  void init();
  void updateInputData();

  ToggleSwitchDriver m_powerSwitch{2};
  ToggleSwitchDriver m_modeSwitch{3};
  PushButtonDriver m_displayButton{8};
  MyEncoderDriver m_myEncoder{6, 7};
  TemperatureSensorDriver m_tempSensor{5};
  ControllerInputData &m_devices_data;
};

class OutputDevices {
public:
  OutputDevices(ControllerOutputIntent &oi);

  void init();
  void update();

  RelaisDriver m_relais{4};
  ControllerOutputIntent &m_devices_intent;
  DisplayDriver m_lcdDisplay; // Keine PIN Zuweisung nötig, da Arduino I2C Bus
                              // automatisch erkennt.
};
