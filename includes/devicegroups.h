#pragma once

#include "interfaces.h"
#include "types.h"

class InputDevices {
public:
  InputDevices(ControllerInputData &inputData, IToggleSwitch &ps,
               IToggleSwitch &ms,
               IPushButton &db, IEncoderDriver &encoderDriver,
               ITempSensorDriver &tempSensorDriver);

  void init();
  void updateInputData();

  ControllerInputData &m_inputData;
private:
  IToggleSwitch &m_powerSwitch;
  IToggleSwitch &m_modeSwitch;
  IPushButton &m_displayButton;
  IEncoderDriver &m_encoderDriver;
  ITempSensorDriver &m_tempSensorDriver;
};

class OutputDevices {
public:
  OutputDevices(ControllerOutputIntent &outputIntent,
                IDisplayDriver &displayDriver,
                IRelais &relais);

  void init();
  void update();

  ControllerOutputIntent &m_outputIntent;
private:
  IRelais &m_relais;
  IDisplayDriver &m_displayDriver;
};
