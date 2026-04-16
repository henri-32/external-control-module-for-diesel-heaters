#pragma once
#include "types.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class DisplayDriver {
  // Config
  // Lib benutzt, keine Pin Zuweisung nötig, da I2C automatisch erkannt
  LiquidCrystal_I2C lcdLibObject{0x27, 20, 4};
  static constexpr uint8_t Rows = 4;
  static constexpr uint8_t Cols = 21;
  char m_lineBuffer[Rows][Cols] = {};
  char string_of_states[Rows][Cols] = {};
  char lastLine[4][21] = {"", "", "", ""};
  // States
  ControllerOutputIntent::DisplayContent &m_displaycontent;
  ControllerOutputIntent::LCD_StateIntent &m_displayState;
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

public:
  DisplayDriver(ControllerOutputIntent::DisplayContent &dc,
                ControllerOutputIntent::LCD_StateIntent &ds);

  void init();
  void update();

  void cyclePages(ControllerOutputIntent::LCD_CycleDirection direction);

private:
  void renderLines();
  void writeDisplay(char lines[4][21]);

  void formatTempFloatsForDisplay(); 
  void createStateStringsForDisplay(
      const ControllerOutputIntent::DisplayContent &content); 
  void clearLine(uint8_t line); 
};
