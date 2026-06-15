#pragma once
#include "config.h"
#include <stdint.h>
#include <limits.h>

//=========================================
//Hier werden die Ein-/ und Ausgabe Structs
//des SystemControllers definiert
//=========================================

struct InputDevicesDataSet {
  struct SwitchAction {
    bool power = false;
    bool mode = false;
  };
  SwitchAction switchAction;

  int8_t encoder_val = 0;
  float sensor_tempC = 0;

  struct Alternator {
    bool pressed = false;
    bool released = false;
    bool used = false;
  };
  Alternator alternator;
};

struct HeaterStatus {
  enum class State { Off, On };
  State state = State::Off;

  enum class Mode { Temp, Power };
  Mode mode = Mode::Temp;

  float target_tempC = Config::kDefaultTempC;
};

#ifdef MEMORY_FUNCTIONS
struct LongtimeData {
  int dutyCycle = 0;
  int avgIdleTime = 0;
};
struct RuntimeData {
  unsigned int dutyCycle = 0;
  unsigned long avgIdleTime_minutes = 0;
  unsigned long maxIdleTime_minutes = 0;
  unsigned int minIdleTime_minutes = UINT_MAX;
  unsigned int cycleCounter = 0;
  float mediumDiffTempToTarget = 0.0;
};

struct CalculationData {
  uint16_t updateCounter = 0;
  float tempDiffAccumulator = 0.0;
  unsigned long lastON = 0;
  unsigned long lastOFF = 0;
  unsigned long accumulatedTimeOFF = 0;
  unsigned long accumulatedTimeON = 0;
  unsigned long lastOFFperiodeLength = 0;
};
#endif

struct OutputDevicesIntent {
public:
  struct DisplayContent {
    float temp_c;
    HeaterStatus status;
#ifdef MEMORY_FUNCTIONS
    RuntimeData runtimeDisplayData;
    LongtimeData EEPROM_Values;
#endif
  };
  DisplayContent displayContent;

  enum class LcdStateIntent { Page1, Page2, Page3, Page4, Off };
  LcdStateIntent lcd_state = LcdStateIntent::Off;

  enum class LcdCycleDirection { None, Right, Left };
  LcdCycleDirection lcd_cycleDirection = LcdCycleDirection::None;

  enum class RelaisCommand { Long, Short, None };
  RelaisCommand relaisCommand = RelaisCommand::None;
};
