#pragma once
#include "interfaces.h"

class DisplayDriver : public IDisplayDriver {
public:
  DisplayDriver(IDisplayHardware &display, OutputDevicesIntent::DisplayContent &dc,
                OutputDevicesIntent::LCD_StateIntent &ds);

  void init() override;
  void update() override;

  void renderLines();
  void writeDisplay(char lines[4][21]);

  void formatTempFloatsForDisplay();
  void createStateStringsForDisplay(
      const OutputDevicesIntent::DisplayContent &content);
  void clearLine(uint8_t line);

  IDisplayHardware &m_display;
  static constexpr uint8_t Rows = 4;
  static constexpr uint8_t Cols = 21;
  char m_lineBuffer[Rows][Cols] = {};
  char string_of_states[Rows][Cols] = {};
  char lastLine[4][21] = {"", "", "", ""};
  // States
  OutputDevicesIntent::DisplayContent &m_displayContent;
  OutputDevicesIntent::LCD_StateIntent &m_displayState;
  // Formatting in Helperfunktion
  int t_int;
  int t_frac;
  int s_int;
  int s_frac;
  int diff_int;
  int diff_frac;
  // Timing
  unsigned long last_update_ms = 0;
  const uint8_t min_update_interval_ms = 100;
};
