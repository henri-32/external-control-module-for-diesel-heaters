#pragma once
#include "types.h"
#include <Arduino.h>
#include <DallasTemperature.h>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>

class ToggleSwitchDriver {
  // Config
  const uint8_t m_pin;
  // Statues
  bool m_current;
  bool m_prev;
  // Timing
  unsigned long m_last_debounce_ms = 0;
  static constexpr unsigned long m_debounce_delay_ms = 50;
  // API
public:
  explicit ToggleSwitchDriver(uint8_t pin) : m_pin(pin){};

  void init() {
    pinMode(m_pin, INPUT_PULLUP);
    m_current = digitalRead(m_pin);
    m_prev = m_current;
  };

  bool changed() {
    m_current = digitalRead(m_pin);
    if (m_current == m_prev)
      return false;
    if (millis() - m_last_debounce_ms < m_debounce_delay_ms)
      return false;

    m_prev = m_current;
    m_last_debounce_ms = millis();
    return true;
  };
};

class PushButtonDriver {
  const uint8_t m_pin;
  bool m_stable;
  bool m_last_read;
  unsigned long m_last_debounce_ms = 0;
  static constexpr unsigned long m_debounce_delay_ms = 50;

public:
  explicit PushButtonDriver(uint8_t pin) : m_pin(pin) {}

  void init() {
    pinMode(m_pin, INPUT_PULLUP);
    m_stable = digitalRead(m_pin);
    m_last_read = m_stable;
  }

  bool isDown() const {
    return m_stable == LOW;
  }

  bool released() {
    bool reading = digitalRead(m_pin);

    if (reading != m_last_read) {
      m_last_debounce_ms = millis();
      m_last_read = reading;
    }

    if (millis() - m_last_debounce_ms < m_debounce_delay_ms)
      return false;

    if (m_stable != reading) {
      m_stable = reading;
      return m_stable == HIGH; // nur bei stabilem LOW-> HIGH
    }

    return false;
  }
};

// Name gewählt, da Bibliothek Encoder heißt.
class MyEncoderDriver {
  // Config
  Encoder m_encoder; // Encoder library genutzt: Dort findet Pin Zuordnung statt
                     // States
  long m_current = 0;
  long m_prev = 0;
  long m_delta = 0;
  // Timing
  unsigned long m_last_change_ms = 0;
  // API
public:
  explicit MyEncoderDriver(uint8_t pin1, uint8_t pin2)
      : m_encoder(pin1, pin2){};
  // Trotz Initialisierung durch Encoder Lib eigene init () Methode, damit
  // optisch schön
  void init() { poll(); }
  // Ermöglicht einzelnen Aufruf, trotz unterschiedlichen Timings der
  // Helperfunktionen
  int readSteps() {
    poll();
    return translateStepsToInput();
  }
  // Helperfunktionen
private:
  void poll() {
    m_current = m_encoder.read();
    long diff = m_current - m_prev;
    if (diff == 0)
      return;
    m_delta += diff;
    m_last_change_ms = millis();
    m_prev = m_current;
  }

  int translateStepsToInput() {
    if (millis() - m_last_change_ms < 100)
      return 0;
    int steps = m_delta / 4; // Wird auf ganze Schritte runtergebrochen und rest
                             // vernichtet bei Ganzzahldivision
    m_delta -= steps * 4;    // Das Delta wird um die Schritte die ganz gemacht
                             // wurden wieder reduziert (in Rohwerten) so bleibt
                             // der Rest für den nächsten Aufruf erhalten
    return steps;
  };
};

class TemperatureSensorDriver {
  // Config
  OneWire m_one_wire; // OneWire Lib genutzt: Dort findet Pin Zuweisung statt
  DallasTemperature m_sensors; // DallasTemperature Lib genutzt für
                               // Temperatursensor Timing
  unsigned long m_last_temp_request = 0;
  bool m_tempRequestPending = false;
  static constexpr unsigned long m_request_intervall_ms = 2000;
  static constexpr unsigned long m_conversion_time_ms = 750;
  // Ergebnis
  float m_temp_c;
  // API
public:
  explicit TemperatureSensorDriver(uint8_t pin)
      : m_one_wire(pin), m_sensors(&m_one_wire) {
  } // Verknüpfung von DallasTemperature & OneWire

  void init() {
    m_sensors.begin();
    m_sensors.requestTemperatures();
    m_temp_c = m_sensors.getTempCByIndex(0);
  }

  float pollTemp() {
    startTemperatureRequest();
    measureTemperature();
    return m_temp_c;
  }
  // Helperfunktionen
private:
  void startTemperatureRequest() {
    if (millis() - m_last_temp_request < m_request_intervall_ms)
      return;
    if (m_tempRequestPending)
      return;
    m_sensors.requestTemperatures();
    m_last_temp_request = millis();
    m_tempRequestPending = true;
  }

  void measureTemperature() {
    if (!m_tempRequestPending)
      return;
    if (millis() - m_last_temp_request < m_conversion_time_ms)
      return;
    m_temp_c = m_sensors.getTempCByIndex(0);
    m_tempRequestPending = false;
  };
};

class RelaisDriver {
  // Config
  const uint8_t m_pin;
  // States
  enum class RelaisState { ON, OFF };
  RelaisState m_relais_state = RelaisState::OFF;
  // Timing
  uint16_t m_pulse_ms = 0;
  unsigned long m_pulse_start_ms = 0;
  // API
public:
  explicit RelaisDriver(const uint8_t pin) : m_pin(pin) {}

  void init() {
    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, LOW);
  }

  void update(ControllerOutputIntent::RelaisCommand intent) {
    if (m_relais_state == RelaisState::OFF) {
      /*muss hier als positiv Bedinung behandelt werden, damit der zweite Teil
       der Funktion (Überprüfung des Ablaufs) unabhängig vom Zustand erfolgt*/
      if (intent == ControllerOutputIntent::RelaisCommand::None)
        return;
      applyPulseLengthFromIntent(intent);
      m_pulse_start_ms = millis();
      activate();
      m_relais_state = RelaisState::ON;
    } else if (m_relais_state == RelaisState::ON) {
      if (millis() - m_pulse_start_ms < m_pulse_ms)
        return;
      deactivate();
      m_relais_state = RelaisState::OFF;
    }
  }

private:
  void applyPulseLengthFromIntent(
      ControllerOutputIntent::RelaisCommand intent_copy) {
    if (intent_copy == ControllerOutputIntent::RelaisCommand::Long)
      m_pulse_ms = 1500;
    else if (intent_copy == ControllerOutputIntent::RelaisCommand::Short)
      m_pulse_ms = 200;
    else if (intent_copy == ControllerOutputIntent::RelaisCommand::None)
      m_pulse_ms = 0;
  }

  void activate() { digitalWrite(m_pin, HIGH); }

  void deactivate() { digitalWrite(m_pin, LOW); };
};

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
  // API
public:
  DisplayDriver(ControllerOutputIntent::DisplayContent &dc,
                ControllerOutputIntent::LCD_StateIntent &ds)
      : m_displaycontent(dc), m_displayState(ds) {}

  void init() {
    Wire.begin();
    lcdLibObject.init();
    lcdLibObject.noBacklight();
    lcdLibObject.noDisplay();
    lcdLibObject.clear();
  }

  void update() {
    if (m_displayState == ControllerOutputIntent::LCD_StateIntent::OFF) {
      lcdLibObject.noBacklight();
      lcdLibObject.noDisplay();
      return;
    }
    renderLines();
    writeDisplay(m_lineBuffer);
  }

  // Helperfunktionen
  void renderLines() {
    switch (m_displayState) {
    case ControllerOutputIntent::LCD_StateIntent::Page1: 
      formatTempFloatsForDisplay();
      createStateStringsForDisplay(m_displaycontent);
      lcdLibObject.backlight();
      lcdLibObject.display();

      snprintf(m_lineBuffer[0], 21, "Temp.:     %d.%d C", t_int, t_frac);
      snprintf(m_lineBuffer[1], 21, "Solltemp.: %d.%d C", s_int, s_frac);
      snprintf(m_lineBuffer[2], 21, "Zustand:   %s", string_of_states[0]);
      snprintf(m_lineBuffer[3], 21, "Mode:      %s", string_of_states[1]);
      break;
    

    case ControllerOutputIntent::LCD_StateIntent::Page2: 
      lcdLibObject.backlight();
      lcdLibObject.display();

      snprintf(m_lineBuffer[0], 21, "DutyCycle: %u %%", m_displaycontent.runtimeDisplayData.dutyCycle);
      snprintf(m_lineBuffer[1], 21, "Cycles:    %u", m_displaycontent.runtimeDisplayData.cycleCounter);
      snprintf(m_lineBuffer[2], 21, "Avg Idle:  %lu m", m_displaycontent.runtimeDisplayData.avgIdleTime_minutes);
      snprintf(m_lineBuffer[3], 21, "%s", "");
      break;
      
    case ControllerOutputIntent::LCD_StateIntent::Page3:
      formatTempFloatsForDisplay();
      lcdLibObject.backlight();
      lcdLibObject.display();

      snprintf(m_lineBuffer[0], 21, "Max Idle:  %lu m", m_displaycontent.runtimeDisplayData.maxIdleTime_minutes);
      snprintf(m_lineBuffer[1], 21, "Min Idle:  %u m", m_displaycontent.runtimeDisplayData.minIdleTime_minutes);
      snprintf(m_lineBuffer[2], 21, "Avg diff:  %d.%d C", diff_int, diff_frac);
      snprintf(m_lineBuffer[3], 21, "%s", "");
      break;

    case ControllerOutputIntent::LCD_StateIntent::Page4:
      lcdLibObject.backlight();
      lcdLibObject.display();

      snprintf(m_lineBuffer[0], 21, "All Time DC %u %%", m_displaycontent.EEPROM_Values.dutyCycle); 
      snprintf(m_lineBuffer[1], 21, "All Time IT %lu m", m_displaycontent.EEPROM_Values.avgIdleTime);
      snprintf(m_lineBuffer[2], 21, "%s", "");
      snprintf(m_lineBuffer[3], 21, "%s", "");
      break;

    case ControllerOutputIntent::LCD_StateIntent::OFF: 
      lcdLibObject.noBacklight();
      lcdLibObject.noDisplay();
      break;
    }
  }

  void writeDisplay(char lines[4][21]) {
    if (millis() - last_update_ms < min_update_interval_ms)
      return;

    for (uint8_t i = 0; i < 4; i++) {
      if (strcmp(lines[i], lastLine[i]) == 0)
        continue;

      clearLine(i);
      lcdLibObject.print(lines[i]);
      strncpy(lastLine[i], lines[i], 21);
      lastLine[i][20] = '\0';
      last_update_ms = millis();
    }
  }

  void formatTempFloatsForDisplay() {
    switch (m_displayState) {
    case ControllerOutputIntent::LCD_StateIntent::Page1:
    
    t_int = int(m_displaycontent.temp_c);
    t_frac = abs(static_cast<int>(m_displaycontent.temp_c * 10) % 10);
    s_int = int(m_displaycontent.target_temp_c);
    s_frac = abs(static_cast<int>(m_displaycontent.target_temp_c * 10) % 10);
    break; 

    case ControllerOutputIntent::LCD_StateIntent::Page3:
      diff_int = int(m_displaycontent.runtimeDisplayData.mediumDiffTempToTarget);
      diff_frac = abs(static_cast<int>(m_displaycontent.runtimeDisplayData.mediumDiffTempToTarget * 10) % 10);
      break;
    case ControllerOutputIntent::LCD_StateIntent::OFF:
      break;
    }
  }

  void createStateStringsForDisplay(
      const ControllerOutputIntent::DisplayContent &content) {
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

  void clearLine(uint8_t line) {
    lcdLibObject.setCursor(0, line);
    lcdLibObject.print("                    ");
    lcdLibObject.setCursor(0, line);
  }

  void cyclePages(ControllerOutputIntent::LCD_CycleDirection direction) {
    if (direction == ControllerOutputIntent::LCD_CycleDirection::right) {
      switch (m_displayState) {
      case ControllerOutputIntent::LCD_StateIntent::Page1:
        m_displayState = ControllerOutputIntent::LCD_StateIntent::Page2;
        break;
      case ControllerOutputIntent::LCD_StateIntent::Page2:
        m_displayState = ControllerOutputIntent::LCD_StateIntent::Page3;
        break;
      case ControllerOutputIntent::LCD_StateIntent::Page3:
        m_displayState = ControllerOutputIntent::LCD_StateIntent::Page4;
        break;
      case ControllerOutputIntent::LCD_StateIntent::Page4:
        m_displayState = ControllerOutputIntent::LCD_StateIntent::Page1;
        break;
      }
      return;
    }
    if (direction == ControllerOutputIntent::LCD_CycleDirection::left) {
      switch (m_displayState) {
      case ControllerOutputIntent::LCD_StateIntent::Page1:
        m_displayState = ControllerOutputIntent::LCD_StateIntent::Page3;
        break;
      case ControllerOutputIntent::LCD_StateIntent::Page2:
        m_displayState = ControllerOutputIntent::LCD_StateIntent::Page1;
        break;
      case ControllerOutputIntent::LCD_StateIntent::Page3:
        m_displayState = ControllerOutputIntent::LCD_StateIntent::Page2;
        break;
      case ControllerOutputIntent::LCD_StateIntent::Page4:
        m_displayState = ControllerOutputIntent::LCD_StateIntent::Page3;
        break;
      }
      return;
    }
  }
};
