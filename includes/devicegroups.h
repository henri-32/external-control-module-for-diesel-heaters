#pragma once
#include "config.h"
#include "display.h"
#include "myEncoder.h"
#include "pushbuttons.h"
#include "relais.h"
#include "toggle_switches.h"
#include "temperature_sensor.h"

#include "interfaces.h"
#include "types.h"

class RealInputDevices : public IInputDevices {
public:
  RealInputDevices(ControllerInputData &id, ToggleSwitch &ps, ToggleSwitch &ms,
                   PushButton &db, IEncoder &encoder);

  void init();
  void updateInputData();

  ToggleSwitch &m_powerSwitch;
  ToggleSwitch &m_modeSwitch;
  PushButton & m_displayButton; 
  TemperatureSensorDriver m_tempSensor{my_pin_config::tempSensor};
  MyEncoder m_myEncoder; //Keine Referenz, da Treiber MyEncoder konstruiert wird hier (Siehe Konstruktor RealInputDevices)
  ControllerInputData &m_devices_data;
};

class RealOutputDevices : public IOutputDevices {
public:
  RealOutputDevices(ControllerOutputIntent &oi, IDisplay &display);

  void init() override;
  void update() override;

  ControllerOutputIntent &m_devices_intent;
  Relais m_relais{my_pin_config::relais};
  Display m_lcdDisplay;
};
