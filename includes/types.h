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
    bool used = false;
  };
  Modifier modifier;
};

struct HeaterStatus {
  enum class State { Off, On };
  State state = State::Off;
  enum class Mode { Temp, Power };
  Mode mode = Mode::Temp;

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
  };

  DisplayContent displayContent;

  enum class LcdStateIntent {
    start_page,
    default_temp_page,
    default_temp_dialog,
    Off
  };
  LcdStateIntent lcd_state = LcdStateIntent::Off;

  enum class LcdCycleDirection { None, Right, Left };
  LcdCycleDirection lcd_cycleDirection = LcdCycleDirection::None;

  enum class RelaisCommand { Long, Short, None };
  RelaisCommand relaisCommand = RelaisCommand::None;
};
