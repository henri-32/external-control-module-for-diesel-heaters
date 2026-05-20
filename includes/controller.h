#pragma once
#include "types.h"

#ifdef MEMORY_FUNCTIONS
#include "memory.h"
#include "statistics.h"
#endif

class IInputDevices; 
class IOutputDevices; 

class SystemController {
public:
  SystemController(IInputDevices &i, IOutputDevices &o);
  void operator()();
  void init();

// Fragwürdig, dass fürs Testen auf Interna zugegriffen werden muss. 
// Da der Umfang des Projekts erstmal begrenzt ist und die Logik der privaten Funktionen
// getestet werden soll, aktuell für mich akzeptiert. 
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

  // Nur implementiert bei ifdef MEMORY_FUNCTIONS
  //  Für stabile API immer deklariert.
  void writeOutputIntent();
  void updateMemory();

  // Helper
  void clampTargetTempC(float &target);
  void cyclePages();
  void requestRelaisCommand(OutputDevicesIntent::RelaisCommand command);

  IInputDevices &inputDevices;
  HeaterStatus heaterStatus;
  IOutputDevices &outputDevices;

#ifdef MEMORY_FUNCTIONS
  SystemStatistics systemStatistic;
  StatisticMemoryController memoryController;
#endif
};
