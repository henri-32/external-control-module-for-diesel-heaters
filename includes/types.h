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
  } switchAction;

  int8_t encoder_val = 0;
  float sensor_tempC = 0;

  struct Modifier {
    bool pressed = false;
    bool released = false;
  } modifier;
};

enum class UiState {
  start_page,
  menu_page,
  default_temp_menu,
  default_temp_dialog,
  short_relais_impulse_menu,
  pulse_dialog,
  Off
};

struct HeaterStatus {
  enum class State { Off, On } state = State::Off;
  enum class Mode { Temp, Power } mode = Mode::Temp;
  UiState uiState = UiState::Off;

  float target_tempC = Config::kDefaultTempC;
};

template <typename T> struct ConfigMember {
  ConfigMember(T val, uint32_t ad) : value(val), addr(ad) {};
  T value;
  uint32_t addr;
};

struct RuntimeConfigData {
  ConfigMember<float> default_tempC{0.0, 0};
  ConfigMember<uint8_t> RelaisPulseShortMs{200,
                                           0 + sizeof(default_tempC.value)};
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
