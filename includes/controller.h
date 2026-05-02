#pragma once

#ifdef MEMORY_FUNCTIONS
#include "memory.h"
#include "statistics.h"
#endif

#ifdef TEST_BUILD
#include "test_devices.h"
#else
#include "devicegroups.h"
#endif
#include "types.h"

class SystemController {
public:
  SystemController(IInputDevices &i, IOutputDevices &o);
  void operator()();
  void init();

// Fragwürdig, dass fürs Testen auf Interna zugegriffen werden muss. 
// Da Umfang des Projekts erstmal begrenzt und die Logik der privaten Funktionen
// getestet werden soll, für mich akzeptiert. 
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

  // Helper
  void clampTargetTempC(float &target);
  void cyclePages();
  void requestRelaisCommand(ControllerOutputIntent::RelaisCommand command,
                            ControllerOutputIntent::RelaisPriority priority);

  ControllerInputData inputData;
  HeaterStatus heaterStatus;
  ControllerOutputIntent outputIntent;
  IInputDevices &inputDevices;
  IOutputDevices &outputDevices;

#ifdef MEMORY_FUNCTIONS
  SystemStatistics systemStatistic;
  StatisticMemoryController memoryController;
#endif
};
