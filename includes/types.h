#pragma once
#include "config.h"
#include <limits.h>
#include <stdint.h>

// Werttypen
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
  enum class HeatingState { OFF, ON };
  HeatingState state = HeatingState::OFF;

  enum class Mode { TEMP, POWER };
  Mode mode = Mode::TEMP;

  float target_tempC = config::defaultTemp;
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
  float TempDiffcontainer = 0.0;
  unsigned long lastON = 0;
  unsigned long lastOFF = 0;
  unsigned long accumulatedTimeOFF = 0;
  unsigned long accumulatedTimeON = 0;
  unsigned long lastOFFperiodeLength = 0;
};
#endif

class OutputDevicesIntent {
public:
  struct DisplayContent {
    float temp_c;
    HeaterStatus status;
#ifdef MEMORYFUNCTIONS
    RuntimeData runtimeDisplayData;
    LongtimeData EEPROM_Values;
#endif
  };
  DisplayContent displayContent;

  enum class LCD_StateIntent { Page1, Page2, Page3, Page4, OFF };
  LCD_StateIntent lcd_state = LCD_StateIntent::OFF;

  enum class LCD_CycleDirection { none, right, left };
  LCD_CycleDirection lcd_cycleDirection = LCD_CycleDirection::none;

  enum class RelaisCommand { Long, Short, None };
  RelaisCommand relaisCommand = RelaisCommand::None;
};
