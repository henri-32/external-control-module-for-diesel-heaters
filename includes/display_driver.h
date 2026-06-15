#pragma once
#include "interfaces.h"

class DisplayDriver : public IDisplayDriver {
//Treiberklasse für das I2C Display. Enthält auch Rendering und Berechnung 
//der darzustellenden Informationen
public:
  DisplayDriver(IDisplayHardware &display, OutputDevicesIntent::DisplayContent &dc,
                OutputDevicesIntent::LcdStateIntent &ds);

  void init() override;
  void update() override;

  void renderLines();
  void writeDisplay(char lines[4][21]);

  void formatTempFloatsForDisplay();
  void createStateStringsForDisplay(
      const OutputDevicesIntent::DisplayContent &content);
  void clearLine(uint8_t line);

  IDisplayHardware &m_display;
  static constexpr uint8_t kRows = 4;
  static constexpr uint8_t kCols = 21;
  char m_lineBuffer[kRows][kCols] = {};
  char string_of_states[kRows][kCols] = {};
  char lastLine[4][21] = {"", "", "", ""};
  // States
  OutputDevicesIntent::DisplayContent &m_displayContent;
  OutputDevicesIntent::LcdStateIntent &m_displayState;
  // Formatting in Helperfunktion
  int t_int;
  int t_frac;
  int s_int;
  int s_frac;
  int diff_int;
  int diff_frac;
  // Timing
  unsigned long last_update_ms = 0;
  static constexpr uint8_t kMinUpdateIntervalMs = Config::kMinLcdUpdateIntervalMs;
};
