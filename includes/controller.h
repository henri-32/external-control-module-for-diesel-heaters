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
  void applyHeatingLogic();
  void writeOutputIntent(); 


  // Helper
  void apply_config_data();
  void clampTempToConfigVals(float &target);
  void cyclePages();
  void cycleMenuPages();
  void requestRelaisCommand(OutputDevicesIntent::RelaisCommand command);

  bool displayButtonAction(); 
  void enter_default_temp_dialog();

  IRuntimeConfig &runtimeConfig;
  IInputDevices &inputDevices;
  HeaterStatus heaterStatus;
  IOutputDevices &outputDevices;

  float pendingDefaultTempC_ = 0;
  bool modifier_used_ = false; 

};
