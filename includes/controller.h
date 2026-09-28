#pragma once
#include "types.h"

class IRuntimeConfig;
class IInputDevices;
class IOutputDevices;

class SystemController {
  // Diese Klasse ist der zentrale Top Level Controller, welcher die gesamte
  // Systemkomposition übernimmt
public:
  SystemController(IRuntimeConfig &c, IInputDevices &i, IOutputDevices &o);
  void operator()();
  void init();
  void applyInputdata();
  void applyPowerSwitchInput();
  void applyModeSwitchInput();
  void applyEncoderInput();
  void applyDisplayButtonInput();
  void applyHeatingLogic();
  void writeOutputIntent(); 


  // Helper
  void apply_config_data();
  void clampTargetTempC(float &target);
  void cyclePages();
  void requestRelaisCommand(OutputDevicesIntent::RelaisCommand command);

  void DisplayButtonTurnsDisplayOff();
  void enter_default_temp_dialog();

  IRuntimeConfig &runtimeConfig;
  IInputDevices &inputDevices;
  HeaterStatus heaterStatus;
  IOutputDevices &outputDevices;

  float pendingDefaultTempC = 0;

};
