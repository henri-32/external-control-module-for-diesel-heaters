#pragma once
#include "config.h"
#include "display.h"
#include "myEncoder.h"
#include "pushbuttons.h"
#include "relais.h"
#include "temperature_sensor.h"
#include "toggle_switches.h"

#include "interfaces.h"
#include "types.h"

class InputDevices {
public:
  InputDevices(ControllerInputData &id, IToggleSwitch &ps, IToggleSwitch &ms,
                   IPushButton &db, IEncoder &encoder, ITempSensor &sensor);

  void init();
  void updateInputData();

  ControllerInputData &m_devices_data;
  IToggleSwitch &m_powerSwitch;
  IToggleSwitch &m_modeSwitch;
  IPushButton &m_displayButton;
  MyEncoder m_myEncoder; // Keine Referenz, da Treiber MyEncoder konstruiert
                         // wird hier (Siehe Konstruktor InputDevices)
  TemperatureSensor m_tempSensor;
};

class OutputDevices {
public:
  OutputDevices(ControllerOutputIntent &oi, IDisplay &display, IRelais& relais);

  void init();
  void update();

  ControllerOutputIntent &m_devices_intent;
  IRelais& m_relais;
  Display m_lcdDisplay;
};
