#pragma once

#include "interfaces.h"
#include "types.h"

class InputDevices {
public:
  InputDevices(InputDevicesDataSet &inputData, IToggleSwitch &ps,
               IToggleSwitch &ms,
               IPushButton &db, IEncoderDriver &encoderDriver,
               ITempSensorDriver &tempSensorDriver);

  void init();
  void update();

  InputDevicesDataSet &data;

private:
  IToggleSwitch &m_powerSwitch;
  IToggleSwitch &m_modeSwitch;
  IPushButton &m_displayButton;
  IEncoderDriver &m_encoderDriver;
  ITempSensorDriver &m_tempSensorDriver;
};

class OutputDevices {
public:
  OutputDevices(OutputDevicesIntent &outputIntent,
                IDisplayDriver &displayDriver,
                IRelais &relais);

  void init();
  void update();

  OutputDevicesIntent &intent;

private:
  IRelais &m_relais;
  IDisplayDriver &m_displayDriver;
};
