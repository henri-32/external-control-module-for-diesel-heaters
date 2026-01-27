#include "classes.h"
#include "Arduino.h"
#include <DallasTemperature.h>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>

// Werttypen
struct InputData {
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

struct DisplayContent {
  float temp_c;
  float target_temp_c;
  HeaterStatus::HeatingState heatingState;
  HeaterStatus::Mode mode;
};

struct OutputIntent {
  enum class RelaisCommand { Long, Short, None };
  RelaisCommand relaisCommand = RelaisCommand::None;

  enum class LCD_StateIntent { ON, OFF };
  LCD_StateIntent lcd_stateIntent = LCD_StateIntent::OFF;

  DisplayContent displayContent;
};

// Hardwareadapter

class ToggleSwitchDriver {
//Config
  const uint8_t m_pin;
//Statues
  bool m_current;
  bool m_prev;
//Timing
  unsigned long m_last_debounce_ms = 0;
  static constexpr unsigned long m_debounce_delay_ms = 50;
//API
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
//Config
  const uint8_t m_pin;
//States
  bool m_current;
  bool m_prev;
//Timing
  unsigned long m_last_debounce_ms = 0;
  static constexpr unsigned long m_debounce_delay_ms = 50;
//API
public:
  explicit PushButtonDriver(uint8_t pin) : m_pin(pin) {}

  void init() {
    pinMode(m_pin, INPUT_PULLUP);
    m_current = digitalRead(m_pin);
    m_prev = m_current;
  }

  bool pressed() {
    m_current = digitalRead(m_pin);
    if (m_current == m_prev)
      return false;
    if (millis() - m_last_debounce_ms < m_debounce_delay_ms)
      return false;
    if (m_current == HIGH)
      return false; // Hier wird im Gegensatz zum ToggleSwitch nur auf das
                    // Drücken reagiert, nicht aufs Loslassen
    m_prev = m_current;
    m_last_debounce_ms = millis();
    return true;
  }
};

// Name gewählt, da Bibliothek Encoder heißt.
class MyEncoderDriver { 
// Config
    Encoder m_encoder; // Encoder library genutzt: Dort findet Pin Zuordnung statt
//States
    long m_current = 0;
    long m_prev = 0;
    long m_delta = 0;
//Timing
    unsigned long m_last_change_ms = 0;
//API
  public:
    explicit MyEncoderDriver(uint8_t pin1, uint8_t pin2)
        : m_encoder(pin1, pin2){};
//Trotz Initialisierung durch Encoder Lib eigene init () Methode, damit optisch schön
  void init() {
    poll();
  }
//Ermöglicht einzelnen Aufruf, trotz unterschiedlichen Timings der Helperfunktionen
  int readSteps() { 
    poll();
    return translateStepsToInput();
  }
  //Helperfunktionen
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
//Config
  OneWire m_one_wire; // OneWire Lib genutzt: Dort findet Pin Zuweisung statt
  DallasTemperature m_sensors; // DallasTemperature Lib genutzt für Temperatursensor
//Timing
  unsigned long m_last_temp_request = 0;
  bool m_tempRequestPending = false;
  static constexpr unsigned long m_request_intervall_ms = 2000;
  static constexpr unsigned long m_conversion_time_ms = 750;
//Ergebnis
float m_temp_c;
//API
public:
  explicit TemperatureSensorDriver(uint8_t pin)
      : m_one_wire(pin), m_sensors(&m_one_wire) {} //Verknüpfung von DallasTemperature & OneWire

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
//Helperfunktionen
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
    // Zeit die Messung DS18B20 braucht
    m_temp_c = m_sensors.getTempCByIndex(0);
    m_tempRequestPending = false;
  };

};

class RelaisDriver {
public:
  explicit RelaisDriver(const uint8_t pin) : m_pin(pin) {}

  void init() {
    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, LOW);
  }

  void update(OutputIntent::RelaisCommand intent) {
    if (m_relais_state == RelaisState::OFF) {
      /*muss hier als positiv Bedinung behandelt werden, damit der zweite Teil
       der Funktion (Überprüfung des Ablaufs) unabhängig vom Zustand erfolgt*/
      if (intent == OutputIntent::RelaisCommand::None)
        return;
      convertRelaisIntentToInt(intent);
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
  void convertRelaisIntentToInt(OutputIntent::RelaisCommand intent_copy) {
    if (intent_copy == OutputIntent::RelaisCommand::Long)
      m_pulse_ms = 2000;
    else if (intent_copy == OutputIntent::RelaisCommand::Short)
      m_pulse_ms = 500;
    else if (intent_copy == OutputIntent::RelaisCommand::None)
      m_pulse_ms = 0;
  }

  void activate() { digitalWrite(m_pin, HIGH); }

  void deactivate() { digitalWrite(m_pin, LOW); };

  const uint8_t m_pin;
  uint16_t m_pulse_ms = 0;
  unsigned long m_pulse_start_ms = 0;
  enum class RelaisState { ON, OFF };
  RelaisState m_relais_state = RelaisState::OFF;
};

class DisplayDriver {
private:
  friend class OutputDevices;

  static constexpr uint8_t Rows = 4;
  static constexpr uint8_t Cols = 21;
  char m_line_buf[Rows][Cols] = {};
  char string_of_states[Rows][Cols] = {};
  int t_int = 0;
  int t_frac = 0;
  int s_int = 0;
  int s_frac = 0;

  DisplayContent &m_displaycontent;
  OutputIntent::LCD_StateIntent m_displaystate;

  LiquidCrystal_I2C lcd{0x27, 20, 4};
  DisplayDriver(DisplayContent &dc, OutputIntent::LCD_StateIntent &ds)
      : m_displaycontent(dc), m_displaystate(ds) {}
  /* // Erläuterung für mich: const weil nur gelesen wird und
                    lebende Daten, die möglichst aktuell sein sollen. Das
                     kleine enum kann stattdessen einfach kopiert werden*/

  // Hilfsfunktionen
  void clearLine(uint8_t line) {
    lcd.setCursor(0, line);
    lcd.print("                    ");
    lcd.setCursor(0, line);
  }

  void convertStatesToStrings(HeaterStatus::HeatingState state,
                              HeaterStatus::Mode mode) {
    switch (state) {
    case HeaterStatus::HeatingState::ON:
      strncpy(string_of_states[0], "ON", 21);
      string_of_states[0][20] = '\0';
      break;
    case HeaterStatus::HeatingState::OFF:
      strncpy(string_of_states[0], "OFF", 21);
      string_of_states[0][20] = '\0';
      break;
    }
    switch (mode) {
    case HeaterStatus::Mode::TEMP:
      strncpy(string_of_states[1], "TEMP", 21);
      string_of_states[0][20] = '\0';
      break;
    case HeaterStatus::Mode::POWER:
      strncpy(string_of_states[1], "POWER", 21);
      string_of_states[0][20] = '\0';
      break;
    }
  }

  void formatTempFloatsForDisplay() {
    t_int = int(m_displaycontent.temp_c);
    t_frac = abs((int)(m_displaycontent.temp_c * 10) % 10);
    s_int = int(m_displaycontent.target_temp_c);
    s_frac = abs((int)(m_displaycontent.target_temp_c * 10) % 10);
  }
  void renderLines() {
    convertStatesToStrings(m_displaycontent.heatingState,
                           m_displaycontent.mode);
    switch (m_displaystate) {
    case OutputIntent::LCD_StateIntent::ON: {
      lcd.backlight();
      lcd.display();
      formatTempFloatsForDisplay();

      snprintf(m_line_buf[0], 21, "Temp.:     %d.%d C", t_int, t_frac);
      snprintf(m_line_buf[1], 21, "Solltemp.: %d.%d C", s_int, s_frac);
      snprintf(m_line_buf[2], 21, "Zustand:   %s", string_of_states[0]);
      snprintf(m_line_buf[3], 21, "Mode:      %s", string_of_states[1]);
      break;
    }

    case OutputIntent::LCD_StateIntent::OFF: {
      lcd.noBacklight();
      lcd.noDisplay();
      break;
    }
    }
  }

  void writeUpdate(char lines[4][21]) {
    static char lastLine[4][21] = {"", "", "", ""}; // nötig für Vergleich
    static long last_update_ms = 0;
    constexpr uint8_t min_update_interval_ms = 100;

    if (millis() - last_update_ms < min_update_interval_ms)
      return;

    for (uint8_t i = 0; i < 4; i++) {
      if (strcmp(lines[i], lastLine[i]) == 0)
        continue;

      clearLine(i);
      lcd.print(lines[i]);
      strncpy(lastLine[i], lines[i], 21);
      lastLine[i][20] = '\0';
      last_update_ms = millis();
    }
  }

public:
  void init() {
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();
  }

  void update() {
    renderLines();
    convertStatesToStrings(m_displaycontent.heatingState,
                           m_displaycontent.mode);
    writeUpdate(m_line_buf);
  }
};

class InputDevices {
private:
  friend class SystemController;
  ToggleSwitchDriver m_powerSwitch{2};
  ToggleSwitchDriver m_modeSwitch{3};
  PushButtonDriver m_displayButton{8};
  MyEncoderDriver m_myEncoder{6, 7};
  TemperatureSensorDriver m_tempSensor{5};

  InputDevices() = default;

  void init() {
    m_powerSwitch.init();
    m_modeSwitch.init();
    m_displayButton.init();
    m_myEncoder.init();
    m_tempSensor.init();
  }

  void poll(InputData &output) {
    output.powerSwitchChanged = m_powerSwitch.changed();
    output.modeSwitchChanged = m_modeSwitch.changed();
    output.displayButtonChanged = m_displayButton.pressed();
    output.encoder_val = m_myEncoder.readSteps();
    output.sensor_tempC = m_tempSensor.pollTemp();
  }
};

class OutputDevices {
private:
  friend class SystemController;
  RelaisDriver m_relais{4};
  OutputIntent &m_outputIntent;
  DisplayDriver m_lcdDisplay; // Keine PIN Zuweisung nötig, da Arduino I2C Bus
                              // automatisch erkennt.
  OutputDevices(OutputIntent &oi)
      : m_outputIntent(oi), m_lcdDisplay(m_outputIntent.displayContent,
                                         m_outputIntent.lcd_stateIntent) {}

  void init() {
    m_relais.init();
    m_lcdDisplay.init();
  }

  void update() {
    m_relais.update(m_outputIntent.relaisCommand);
    m_lcdDisplay.update();
    resetHandledOutputIntent();
  }

  void resetHandledOutputIntent() {
    m_outputIntent.relaisCommand = OutputIntent::RelaisCommand::None;
  }
};

class SystemController {
private:
  InputDevices inputDevices;
  InputData inputData;
  HeaterStatus heaterStatus;
  OutputIntent outputIntent;
  OutputDevices outputDevices{outputIntent};

  public:
  SystemController() = default;

  void operator()() {
    inputDevices.poll(inputData);
    applyInputdata();
    applyHeatingLogic();
    writeStatesToOutputIntent();
    outputDevices.update();
  }

  void init() {
    delay(500); // Damit sich Hardware kurz einpendeln kann
    inputDevices.init();
    outputDevices.init();
  }

private:
  void applyInputdata() {
    /* Der Heizungsmodus wird beim OnOff Schalter immer auf POWER gewechselt, um
    die Temperaturlogik daran zu hindern, direkt zurückzuschalten. Das ersetzt
    meine alte Temperatursperre. Das ist unproblematisch weil der Modus
    unabhängig vom Zustand per modeSwitch gewechselt werden kann. Und es ist
    nötig, damit ich beim Verlassen des Bootes, die Heizung aus machen kann und
    sie korrekt herunterfährt, bevor ich den Strom wegnehme*/
    constexpr float TempStep = 0.5;
    constexpr float TempMin = 5.0;
    constexpr float TempMax = 30.0;

    // Power Switch Logik
    if (inputData.powerSwitchChanged) {

      if (heaterStatus.heatingState == HeaterStatus::HeatingState::ON) {
        outputIntent.relaisCommand = OutputIntent::RelaisCommand::Long;
        heaterStatus.heatingState = HeaterStatus::HeatingState::OFF;
        heaterStatus.mode = HeaterStatus::Mode::POWER;

      } else {
        outputIntent.relaisCommand = OutputIntent::RelaisCommand::Long;
        heaterStatus.heatingState = HeaterStatus::HeatingState::ON;
        heaterStatus.mode = HeaterStatus::Mode::POWER;
      }
    }

    // Mode Switch Logik
    if (inputData.modeSwitchChanged) {

      if (heaterStatus.mode == HeaterStatus::Mode::POWER) {
        outputIntent.relaisCommand = OutputIntent::RelaisCommand::Short;
        heaterStatus.mode = HeaterStatus::Mode::TEMP;
      } else {
        outputIntent.relaisCommand = OutputIntent::RelaisCommand::Short;
        heaterStatus.mode = HeaterStatus::Mode::POWER;
      }
    }

    // Encoder Logik
    //  Hier wird der Modus geprüft, weil der Drehregler mit Knopf vllt mal die
    //  Ansicht auf Laufzeitdaten wechseln soll.
    if (inputData.encoder_val != 0) {

      if (heaterStatus.mode == HeaterStatus::Mode::TEMP) {
        heaterStatus.target_temp_c +=
            inputData.encoder_val * TempStep; // encoderVal ist signed
        if (heaterStatus.target_temp_c > TempMax)
          heaterStatus.target_temp_c = TempMax;
        else if (heaterStatus.target_temp_c < TempMin)
          heaterStatus.target_temp_c = TempMin;
      }
    }

    // Display Switch Logik
    if (inputData.displayButtonChanged) {

      if (outputIntent.lcd_stateIntent == OutputIntent::LCD_StateIntent::ON)
        outputIntent.lcd_stateIntent = OutputIntent::LCD_StateIntent::OFF;
      else {
        outputIntent.lcd_stateIntent = OutputIntent::LCD_StateIntent::ON;
      }
    }
  }

  // Temperaturregelung
  void applyHeatingLogic() {
    constexpr float Solltoleranz = 1.5;

    if (heaterStatus.mode != HeaterStatus::Mode::TEMP)
      return;
    if (inputData.sensor_tempC < heaterStatus.target_temp_c - Solltoleranz &&
        heaterStatus.heatingState == HeaterStatus::HeatingState::OFF) {
      outputIntent.relaisCommand = OutputIntent::RelaisCommand::Long;
      heaterStatus.heatingState = HeaterStatus::HeatingState::ON;
    } else if (inputData.sensor_tempC >
                   heaterStatus.target_temp_c + Solltoleranz &&
               heaterStatus.heatingState == HeaterStatus::HeatingState::ON) {
      outputIntent.relaisCommand = OutputIntent::RelaisCommand::Long;
      heaterStatus.heatingState = HeaterStatus::HeatingState::OFF;
    }
  }

  void writeStatesToOutputIntent() {
    outputIntent.displayContent.temp_c = inputData.sensor_tempC;
    outputIntent.displayContent.target_temp_c = heaterStatus.target_temp_c;
    outputIntent.displayContent.heatingState = heaterStatus.heatingState;
    outputIntent.displayContent.mode = heaterStatus.mode;
  }
};

/*Dann in main.ino bzw main.cpp
SystemController controller;

void setup() { controller.init(); };

void loop() { controller(); }*/
