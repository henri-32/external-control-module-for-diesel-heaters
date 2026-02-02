#pragma once
#include <Arduino.h>
#include <limits.h>

// Werttypen
struct ControllerInputData {
  bool powerSwitchChanged = false;
  bool modeSwitchChanged = false;
  bool displayButtonReleased = false;
  int8_t encoder_val = 0;
  float sensor_tempC = 0;
  bool alternatorPressed = false;
  bool alternatorUsed = false;
};

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

struct HeaterStatus {
  enum class HeatingState { ON, OFF };
  HeatingState heatingState = HeatingState::OFF;

  enum class Mode { TEMP, POWER };
  Mode mode = Mode::TEMP;

  float target_temp_c = 20;
};

class ControllerOutputIntent {
public:
  struct DisplayContent {
    float temp_c;
    float target_temp_c;
    HeaterStatus::HeatingState heatingState;
    HeaterStatus::Mode mode;
    RuntimeData runtimeDisplayData;
  };
  DisplayContent displayContent;

  enum class LCD_StateIntent { Page1, Page2, Page3, OFF };
  LCD_StateIntent lcd_stateIntent = LCD_StateIntent::OFF;

  enum class LCD_CycleDirection {none, right, left};
  LCD_CycleDirection  lcd_cycleDirection = LCD_CycleDirection::none;

  enum class RelaisCommand { Long, Short, None };
  RelaisCommand relaisCommand = RelaisCommand::None;

  enum RelaisPriority { Low, High };

  RelaisPriority m_currentPriority = Low;
  RelaisCommand m_command = RelaisCommand::None;

  void requestRelaisCommand(RelaisCommand command,
                            RelaisPriority priority = RelaisPriority::Low) {
    if (priority >= m_currentPriority) {
      m_command = command;
      m_currentPriority = priority;
    }
  }

  RelaisCommand consumeRelaisRequest() {
    RelaisCommand command = m_command;
    m_command = RelaisCommand::None;
    m_currentPriority = Low;
    return command;
  };
};
