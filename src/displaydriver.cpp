#include "displaydriver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using COI = ControllerOutputIntent; 
using LCDIntent = ControllerOutputIntent::LCD_StateIntent;

//Stubs für Arduino Zugriffe
#ifdef TEST_BUILD
static unsigned long millis() {
  static unsigned long now = 1000;
  now += 1000;
  return now;
}

#else
#include <Arduino.h>
#endif

DisplayDriver::DisplayDriver(IDisplay &display,
                             COI::DisplayContent &dc,
                             LCDIntent &ds)
    : m_display(display), m_displaycontent(dc), m_displayState(ds) {}

void DisplayDriver::init() {
  //{{{
  m_display.init();
  m_display.noBacklight();
  m_display.noDisplay();
  m_display.clear();
}
//}}}

void DisplayDriver::update() {
  //{{{
  if (m_displayState == LCDIntent::OFF) {
    m_display.noBacklight();
    m_display.noDisplay();
    return;
  }
  renderLines();
  writeDisplay(m_lineBuffer);
}
//}}}

void DisplayDriver::renderLines() {
  //{{{
  switch (m_displayState) {
  case LCDIntent::Page1:
    formatTempFloatsForDisplay();
    createStateStringsForDisplay(m_displaycontent);
    m_display.backlight();
    m_display.display();

    snprintf(m_lineBuffer[0], 21, "Temp.:     %d.%d C", t_int, t_frac);
    snprintf(m_lineBuffer[1], 21, "Solltemp.: %d.%d C", s_int, s_frac);
    snprintf(m_lineBuffer[2], 21, "Zustand:   %.9s", string_of_states[0]);
    snprintf(m_lineBuffer[3], 21, "Mode:      %.9s", string_of_states[1]);
    break;

  case LCDIntent::Page2:
    m_display.backlight();
    m_display.display();
	#ifdef MEMORY_FUNCTIONS
    snprintf(m_lineBuffer[0], 21, "DutyCycle: %u %%",
             m_displaycontent.runtimeDisplayData.dutyCycle);
    snprintf(m_lineBuffer[1], 21, "Cycles:    %u",
             m_displaycontent.runtimeDisplayData.cycleCounter);
    snprintf(m_lineBuffer[2], 21, "Avg Idle:  %lu m",
             m_displaycontent.runtimeDisplayData.avgIdleTime_minutes);
    snprintf(m_lineBuffer[3], 21, "%s", "");
	#endif
    break;
  case LCDIntent::Page3:
    formatTempFloatsForDisplay();
    m_display.backlight();
    m_display.display();
	#ifdef MEMORY_FUNCTIONS
    snprintf(m_lineBuffer[0], 21, "Max Idle:  %lu m",
             m_displaycontent.runtimeDisplayData.maxIdleTime_minutes);
    snprintf(m_lineBuffer[1], 21, "Min Idle:  %u m",
             m_displaycontent.runtimeDisplayData.minIdleTime_minutes);
    snprintf(m_lineBuffer[2], 21, "Avg diff:  %d.%d C", diff_int, diff_frac);
    snprintf(m_lineBuffer[3], 21, "%s", "");
	#endif
    break;
  case LCDIntent::Page4:
    m_display.backlight();
    m_display.display();

	#ifdef MEMORY_FUNCTIONS
    snprintf(m_lineBuffer[0], 21, "All Time DC %u %%",
             m_displaycontent.EEPROM_Values.dutyCycle);
    snprintf(m_lineBuffer[1], 21, "All Time IT %lu m",
             m_displaycontent.EEPROM_Values.avgIdleTime);
    snprintf(m_lineBuffer[2], 21, "%s", "");
    snprintf(m_lineBuffer[3], 21, "%s", "");
	#endif
    break;
  case LCDIntent::OFF:
    m_display.noBacklight();
    m_display.noDisplay();
    break;
  }
}
//}}}

void DisplayDriver::writeDisplay(char lines[4][21]) {
  //{{{
  if (millis() - last_update_ms < min_update_interval_ms)
    return;

  for (uint8_t i = 0; i < 4; i++) {
    if (strcmp(lines[i], lastLine[i]) == 0)
      continue;

    clearLine(i);
    m_display.printstr(lines[i]);
    strncpy(lastLine[i], lines[i], 21);
    lastLine[i][20] = '\0';
    last_update_ms = millis();
  }
}
//}}}

void DisplayDriver::formatTempFloatsForDisplay() {
  //{{{
  switch (m_displayState) {
  case LCDIntent::Page1:

    t_int = int(m_displaycontent.temp_c);
    t_frac = abs(static_cast<int>(m_displaycontent.temp_c * 10) % 10);
    s_int = int(m_displaycontent.target_tempC);
    s_frac = abs(static_cast<int>(m_displaycontent.target_tempC * 10) % 10);
    break;

  case LCDIntent::Page2:
    break;

  case LCDIntent::Page3:
    diff_int = int(m_displaycontent.runtimeDisplayData.mediumDiffTempToTarget);
    diff_frac = abs(
        static_cast<int>(
            m_displaycontent.runtimeDisplayData.mediumDiffTempToTarget * 10) %
        10);
    break;

  case LCDIntent::Page4:
    break;

  case LCDIntent::OFF:
    break;
  }
}
//}}}

void DisplayDriver::createStateStringsForDisplay(
    const COI::DisplayContent &content) {
  //{{{
  switch (content.heatingState) {
  case HeaterStatus::HeatingState::ON:
    strncpy(string_of_states[0], "ON", 21);
    string_of_states[0][20] = '\0';
    break;
  case HeaterStatus::HeatingState::OFF:
    strncpy(string_of_states[0], "OFF", 21);
    string_of_states[0][20] = '\0';
    break;
  }
  switch (content.mode) {
  case HeaterStatus::Mode::TEMP:
    strncpy(string_of_states[1], "TEMP", 21);
    string_of_states[1][20] = '\0';
    break;
  case HeaterStatus::Mode::POWER:
    strncpy(string_of_states[1], "POWER", 21);
    string_of_states[1][20] = '\0';
    break;
  }
}
//}}}

void DisplayDriver::clearLine(uint8_t line) {
  //{{{
  m_display.setCursor(0, line);
  m_display.printstr("                    ");
  m_display.setCursor(0, line);
}
//}}}
