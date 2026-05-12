#pragma once

#include "interfaces.h"
#include "types.h"

class InputDevices : public IInputDevices {
public:
  explicit InputDevices(InputDevicesDataSet &inputData, IToggleSwitch &ps,
                        IToggleSwitch &ms, IPushButton &db,
                        IEncoderDriver &encoderDriver,
                        ITempSensorDriver &tempSensorDriver);

  void init() override;
  void update() override;

private:
  IToggleSwitch &m_powerSwitch;
  IToggleSwitch &m_modeSwitch;
  IPushButton &m_displayButton;
  IEncoderDriver &m_encoderDriver;
  ITempSensorDriver &m_tempSensorDriver;
};

class OutputDevices : public IOutputDevices {
public:
  OutputDevices(OutputDevicesIntent &outputIntent,
                IDisplayDriver &displayDriver, IRelais &relais);

  void init() override;
  void update() override;

private:
  IRelais &m_relais;
  IDisplayDriver &m_displayDriver;
};
