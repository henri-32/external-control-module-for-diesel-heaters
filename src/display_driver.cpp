#include "display_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using ODI = OutputDevicesIntent;
using LCDIntent = UiState;

#ifdef TEST_BUILD
#include "ArduinoStubs.h"

#else
#include <Arduino.h>
#endif

DisplayDriver::DisplayDriver(IDisplayHardware &display) : m_display(display) {}

void DisplayDriver::init() {
  //{{{
  m_display.init();
  m_display.noBacklight();
  m_display.noDisplay();
  m_display.clear();
}
//}}}

void DisplayDriver::update(OutputDevicesIntent::DisplayContent content) {
  //{{{
  if (content.status.uiState == LCDIntent::Off) {
    m_display.noBacklight();
    m_display.noDisplay();
    return;
  }
  m_displayContent = content;
  renderLines();
  writeDisplay(m_lineBuffer);
}
//}}}

void DisplayDriver::renderLines() {
  //{{{
  switch (m_displayContent.status.uiState) {
  case LCDIntent::start_page:
    formatTempFloatsForDisplay();
    createStateStringsForDisplay(m_displayContent);
    m_display.backlight();
    m_display.display();

    snprintf(m_lineBuffer[0], 21, "Temp.:     %d.%d C", t_int, t_frac);
    snprintf(m_lineBuffer[1], 21, "Solltemp.: %d.%d C", s_int, s_frac);
    snprintf(m_lineBuffer[2], 21, "Zustand:   %.9s", string_of_states[0]);
    snprintf(m_lineBuffer[3], 21, "Mode:      %.9s", string_of_states[1]);
    break;

  case LCDIntent::default_temp_page:
    m_display.backlight();
    m_display.display();
    snprintf(m_lineBuffer[0], 21, "Default Temp %.2f C",
             m_displayContent.runtimeConfigData.default_tempC);

    snprintf(m_lineBuffer[1], 21, "Press Mode Button");
    snprintf(m_lineBuffer[2], 21, "     to change      ");
    break;

  case LCDIntent::default_temp_dialog:
    m_display.backlight();
    m_display.display();
    snprintf(m_lineBuffer[0], 21, "Default Temp %.2f C",
             m_displayContent.runtimeConfigData.default_tempC);
    snprintf(m_lineBuffer[1], 21, "Mode Button: OK");
    snprintf(m_lineBuffer[2], 21, "Display Button: EXIT");
    break;

  case LCDIntent::Off:
    m_display.noBacklight();
    m_display.noDisplay();
    break;
  }
}
//}}}

void DisplayDriver::writeDisplay(char lines[4][21]) {
  //{{{
  if (millis() - last_update_ms < kMinUpdateIntervalMs)
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
  t_int = int(m_displayContent.temp_c);
  t_frac = abs(static_cast<int>(m_displayContent.temp_c * 10) % 10);
  s_int = int(m_displayContent.status.target_tempC);
  s_frac =
      abs(static_cast<int>(m_displayContent.status.target_tempC * 10) % 10);
}

void DisplayDriver::createStateStringsForDisplay(
    const ODI::DisplayContent &content) {
  //{{{
  switch (content.status.state) {
  case HeaterStatus::State::On:
    strncpy(string_of_states[0], "ON", 21);
    string_of_states[0][20] = '\0';
    break;
  case HeaterStatus::State::Off:
    strncpy(string_of_states[0], "OFF", 21);
    string_of_states[0][20] = '\0';
    break;
  }
  switch (content.status.mode) {
  case HeaterStatus::Mode::Temp:
    strncpy(string_of_states[1], "TEMP", 21);
    string_of_states[1][20] = '\0';
    break;
  case HeaterStatus::Mode::Power:
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
