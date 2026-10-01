#pragma once
#include "config.h"
#include <limits.h>
#include <stdint.h>

//=========================================
// Hier werden die Ein-/ und Ausgabe Structs
// des SystemControllers definiert
//=========================================

struct InputDevicesDataSet {
  struct SwitchAction {
    bool power = false;
    bool mode = false;
  };
  SwitchAction switchAction;

  int8_t encoder_val = 0;
  float sensor_tempC = 0;

  struct Modifier {
    bool pressed = false;
    bool released = false;
  };
  Modifier modifier;
};

enum class UiState {
  start_page,
  default_temp_page,
  default_temp_dialog,
  Off
};

struct HeaterStatus {
  enum class State { Off, On } state = State::Off;
  enum class Mode { Temp, Power } mode = Mode::Temp;
  UiState uiState = UiState::Off; 

  float target_tempC = Config::kDefaultTempC;
};

struct RuntimeConfigData {
  float default_tempC = 15.0;
};

struct OutputDevicesIntent {
public:
  struct DisplayContent {
    float temp_c;
    HeaterStatus status;
    RuntimeConfigData runtimeConfigData;
  } displayContent;

  enum class LcdCycleDirection {
    None,
    Right,
    Left
  } lcd_cycleDirection = LcdCycleDirection::None;

  enum class RelaisCommand {
    Long,
    Short,
    None
  } relaisCommand = RelaisCommand::None;
};
