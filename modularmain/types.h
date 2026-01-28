#pragma once
#include <Arduino.h>

// Werttypen
struct ControllerInputData {
  bool powerSwitchChanged;
  bool modeSwitchChanged;
  bool displayButtonChanged;
  int8_t encoder_val;
  float sensor_tempC;
};

struct HeaterStatus {
  enum class HeatingState { ON, OFF };
  HeatingState heatingState = HeatingState::OFF;

  enum class Mode { TEMP, POWER };
  Mode mode = Mode::TEMP;

  float target_temp_c = 20;
};

struct ControllerOutputIntent {
  struct DisplayContent {
    float temp_c;
    float target_temp_c;
    HeaterStatus::HeatingState heatingState;
    HeaterStatus::Mode mode;
  };
  DisplayContent displayContent;

  enum class RelaisCommand { Long, Short, None };
  RelaisCommand relaisCommand = RelaisCommand::None;

  enum class LCD_StateIntent { ON, OFF };
  LCD_StateIntent lcd_stateIntent = LCD_StateIntent::OFF;
};
