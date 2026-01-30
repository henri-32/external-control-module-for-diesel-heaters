#pragma once
#include <Arduino.h>

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

class HeaterStatus {
public:
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
  };
  DisplayContent displayContent;

  enum class LCD_StateIntent { Page1, Page2, OFF };
  LCD_StateIntent lcd_stateIntent = LCD_StateIntent::OFF;

  enum class RelaisCommand { Long, Short, None };
  RelaisCommand relaisCommand = RelaisCommand::None;

  enum RelaisPriority { Low, High };

  RelaisPriority m_currentPriority = Low;
  RelaisCommand m_command;

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
