#pragma once

#ifdef MEMORY_FUNCTIONS
#include "memory.h"
#include "statistics.h"
#endif

#ifdef TEST_BUILD
#include "../tests/test_devices.h"
#else
#include "devicegroups.h"
#endif
#include "types.h"

class SystemController {
public:
  SystemController() = default;
  void operator()();
  void init();

#ifdef TEST_BUILD
public:
#else
private:
#endif

  void applyInputdata();
  void applyPowerSwitchInput();
  void applyModeSwitchInput();
  void applyEncoderInput();
  void applyDisplayButtonInput();
  void applyHeatingLogic();

  // Nur implementiert ifdef MEMORY_FUNCTIONS
  //  Für stabile API immer deklariert.
  void writeOutputIntent();
  void updateMemory();

  //Helper
  void clampTargetTempC(float& target);
  void cyclePages();

  ControllerInputData inputData;
  HeaterStatus heaterStatus;
  ControllerOutputIntent outputIntent;

#ifdef TEST_BUILD 
  TestInputDevices inputDevices {inputData}; 
  TestOutputDevices outputDevices {outputIntent};
#else
  RealInputDevices inputDevices{inputData};
  RealOutputDevices outputDevices{outputIntent};
#endif

#ifdef MEMORY_FUNCTIONS
  SystemStatistics systemStatistic;
  StatisticMemoryController memoryController;
#endif
};
