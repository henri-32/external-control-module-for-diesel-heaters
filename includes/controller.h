#pragma once
#include "config.h"

#ifdef MEMORY_FUNCTIONS
#include "memory.h"
#include "statistics.h"
#endif

#include "devicegroups.h"
#include "types.h"

class SystemController {
public:
  SystemController() = default;
  void operator()();
  void init();

private:
  void applyInputdata();
  void applyPowerSwitchInput();
  void applyModeSwitchInput();
  void applyEncoderInput();
  void applyDisplayButtonInput();
  void applyHeatingLogic();

  // Nur implementiert ifdef MEMORY_FUNCTIONS
  //  Für stabile API immer deklariert.
  void updateOutputIntent();
  void updateMemory();

  ControllerInputData inputData;
  InputDevices inputDevices{inputData};
  HeaterStatus heaterStatus;
  ControllerOutputIntent outputIntent;
  OutputDevices outputDevices{outputIntent};

#ifdef MEMORY_FUNCTIONS
  SystemStatistics systemStatistic;
  StatisticMemoryController memoryController;
#endif
};
