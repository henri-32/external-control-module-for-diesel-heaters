#include "classes.h"
#include "Arduino.h"
#include <DallasTemperature.h>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>

// Werttypen
struct InputData {
  bool powerSwitchHasChanged;
  bool modeSwitchHasChanged;
  bool displayButtonHasChanged;
  int8_t encoderVal;
  float DS18B20_tempC;
};

struct HeaterStatus {
  enum class HeatingState { ON, OFF };
  HeatingState heatingstate;

  enum class Mode { TEMP, POWER };
  Mode mode;

  float Solltemperatur = 20;
};

struct Display_content {
  float tempC;
  float Solltemperatur;
  HeaterStatus::HeatingState heatingstate;
  HeaterStatus::Mode mode;
};

struct OutputIntent {
  enum class RelaisIntent { long_, short_, neutral };
  RelaisIntent relais_intent = RelaisIntent::neutral;

  enum class LCD_Display_state { ON, OFF };
  LCD_Display_state lcd_display_state = LCD_Display_state::OFF;

  Display_content display_content;
};

// Hardwareadapter

class ToggleSwitchDriver {
public:
  explicit ToggleSwitchDriver(uint8_t pin) : m_pin(pin){};

  void init() {
    pinMode(m_pin, INPUT_PULLUP);
    m_switchVal = digitalRead(m_pin);
    m_lastSwitchVal = m_switchVal;
  };

  bool hasChanged() {
    m_switchVal = digitalRead(m_pin);
    if (m_switchVal == m_lastSwitchVal)
      return false;
    if (millis() - m_lastDebounce < m_debounceDelay)
      return false;
    m_lastSwitchVal = m_switchVal;
    m_lastDebounce = millis();
    return true;
  };

private:
  const uint8_t m_pin;
  bool m_switchVal;
  bool m_lastSwitchVal;
  unsigned long m_lastDebounce = 0;
  const unsigned long m_debounceDelay = 50;
};

class PushButtonDriver {
public:
  explicit PushButtonDriver(uint8_t pin) : m_pin(pin) {}
  void init() {
    pinMode(m_pin, INPUT_PULLUP);
    m_buttonVal = digitalRead(m_pin);
    m_lastButtonVal = m_buttonVal;
  }

  bool isPressed() {
    m_buttonVal = digitalRead(m_pin);
    if (m_buttonVal == m_lastButtonVal)
      return false;
    if (millis() - m_lastDebounce < m_debounceDelay)
      return false;
    if (m_buttonVal == HIGH)
      return false; // Hier wird im Gegensatz zum ToggleSwitch nur auf das
                    // Drücken reagiert, nicht aufs Loslassen
    m_lastButtonVal = m_buttonVal;
    m_lastDebounce = millis();
    return true;
  }

private:
  const uint8_t m_pin;
  bool m_buttonVal;
  bool m_lastButtonVal;
  bool m_lastDebounce = 0;
  const bool m_debounceDelay = 50;
};

class MyEncoderDriver { // Name gewählt, da Bibliothek Encoder heißt.
public:
  explicit MyEncoderDriver(uint8_t pin1, uint8_t pin2)
      : m_encoder(pin1, pin2){};

private:
  void poll() {
    long value = m_encoder.read();
    long diff = value - m_lastValue;
    if (diff == 0)
      return;
    m_delta += diff;
    m_lastAction = millis();
    m_lastValue = value;
  }

  int translateStepsToInput() {
    if (millis() - m_lastAction < 100)
      return 0;
    int steps = m_delta / 4; // Wird auf ganze Schritte runtergebrochen und rest
                             // vernichtet bei Ganzzahldivision
    m_delta -= steps * 4;    // Das Delta wird um die Schritte die ganz gemacht
                             // wurden wieder reduziert (in Rohwerten) so bleibt
                             // der Rest für den nächsten aufruf erhalten
    return steps;
  };

public:
  int updateVal() { // So kann nur ein Aufruf erfolgen, allerdings bleibt die
                    // Refreshrate der beiden Funktionen unterschiedlich
    poll();
    return translateStepsToInput();
  }

  void init() { poll(); }

private:
  Encoder m_encoder; // Encoder library genutzt
  long m_lastValue = 0;
  long m_delta = 0;
  unsigned long m_lastAction = 0;
};

class TemperatureSensorDriver {
public:
  explicit TemperatureSensorDriver(uint8_t pin)
      : m_oneWire(pin), m_sensors(&m_oneWire) {}

  void init() {
    m_sensors.begin();
    m_sensors.requestTemperatures();
    m_tempC = m_sensors.getTempCByIndex(0);
  }

  float updateTemp() {
    startTemperatureRequest();
    measureTemperature();
    return m_tempC;
  }

private:
  void startTemperatureRequest() {
    if (millis() - m_lastTempRequest < m_REQUEST_INTERVAL_MS)
      return;
    if (m_tempRequestPending)
      return;
    m_sensors.requestTemperatures();
    m_lastTempRequest = millis();
    m_tempRequestPending = true;
  }

  void measureTemperature() {
    if (!m_tempRequestPending)
      return;
    if (millis() - m_lastTempRequest < m_CONVERSION_TIME_MS)
      return;
    // Zeit die Messung DS18B20 braucht
    m_tempC = m_sensors.getTempCByIndex(0);
    m_tempRequestPending = false;
  };

private:
  OneWire m_oneWire;
  DallasTemperature m_sensors;
  float m_tempC;
  unsigned long m_lastTempRequest = 0;
  bool m_tempRequestPending = false;
  static constexpr unsigned long m_REQUEST_INTERVAL_MS = 2000;
  // Static heißt hier alle Objekte nutzen die gleiche Variable
  static constexpr unsigned long m_CONVERSION_TIME_MS = 750;
};

class RelaisDriver {
public:
  uint16_t m_relais_intent_int = 0;

  explicit RelaisDriver(const uint8_t pin) : m_pin(pin) {}

public:
  void init() {
    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, LOW);
  }

  void update(OutputIntent::RelaisIntent intent) {
    if (m_relaisState == RelaisState::OFF) {
      if (intent != OutputIntent::RelaisIntent::neutral) {
        m_lastRelaisActivation = millis();
        activate();
        m_relaisState = RelaisState::ON;
      }
    };
    if (m_relaisState == RelaisState::ON) {
      convertRelaisIntentToInt(intent);
      if (millis() - m_lastRelaisActivation < m_relais_intent_int)
        return;
      deactivate();
      m_relaisState = RelaisState::OFF;
    }
  }

private:
  void convertRelaisIntentToInt(OutputIntent::RelaisIntent l_intent) {
    if (l_intent == OutputIntent::RelaisIntent::long_)
      m_relais_intent_int = 2000;
    else if (l_intent == OutputIntent::RelaisIntent::short_)
      m_relais_intent_int = 500;
    else if (l_intent == OutputIntent::RelaisIntent::neutral)
      m_relais_intent_int = 0;
  }
  void activate() {
    digitalWrite(m_pin, HIGH);
    m_lastRelaisActivation = millis();
  }

  void deactivate() { digitalWrite(m_pin, LOW); };

  const uint8_t m_pin;
  unsigned long m_lastRelaisActivation = 0;
  enum class RelaisState { ON, OFF };
  RelaisState m_relaisState = RelaisState::OFF;
};

class DisplayDriver {
private:
  friend class OutputDevices;
  static constexpr uint8_t Rows = 4;
  static constexpr uint8_t Cols = 20;
  char content[Rows][Cols] = {};
  char stringOfStates[Rows][Cols] = {};
  const Display_content &m_displaycontent;
  OutputIntent::LCD_Display_state m_displaystate;

  LiquidCrystal_I2C lcd{0x27, 20, 4};
  DisplayDriver(const Display_content &dc,
                OutputIntent::LCD_Display_state
                    ds) // Erläuterung für mich const weil nur gelesen wird und
                        // lebende Daten, die möglichst aktuell seine solen. Das
                        // kleine enum kann einfach kopiert werden
      : m_displaycontent(dc), m_displaystate(ds) {}

  // Hilfsfunktion leeren der Zeile
  void clearLine(uint8_t line) {
    lcd.setCursor(0, line);
    lcd.print("                    ");
    lcd.setCursor(0, line);
  }

  // Hilfsfunktion: Zustände werden in Strings umgewandelt um sie auf dem
  // Display anzuzeigen zu können
  void statesToStrings(HeaterStatus::HeatingState state,
                       HeaterStatus::Mode mode) {
    switch (state) {
    case HeaterStatus::HeatingState::ON:
      strncpy(stringOfStates[0], "ON", 21);
      stringOfStates[0][20] = '\0';
      break;
    case HeaterStatus::HeatingState::OFF:
      strncpy(stringOfStates[0], "OFF", 21);
      stringOfStates[0][20] = '\0';
      break;
    }
    switch (mode) {
    case HeaterStatus::Mode::TEMP:
      strncpy(stringOfStates[1], "TEMP", 21);
      stringOfStates[0][20] = '\0';
      break;
    case HeaterStatus::Mode::POWER:
      strncpy(stringOfStates[1], "POWER", 21);
      stringOfStates[0][20] = '\0';
      break;
    }
  }

  // Inhaltsfunktion die berechnet, was angezeigt wird (content)

  void createDisplayContent() {
    statesToStrings(
        m_displaycontent.heatingstate,
        m_displaycontent
            .mode); // Es werden die aktuellen globalen Zustände eingelesen
    // und in Strings umgewandelt
    switch (m_displaystate) {
    case OutputIntent::LCD_Display_state::ON: {
      lcd.backlight();
      lcd.display();
      int t_int = int(m_displaycontent.tempC); 
      int t_frac = abs((int)(m_displaycontent.tempC*10) % 10);
      int s_int = int(m_displaycontent.Solltemperatur);
      int s_frac = abs((int)(m_displaycontent.Solltemperatur * 10) % 10);

      snprintf(content[0], 21, "Temp.:     %d.%d C", t_int, t_frac);
      snprintf(content[1], 21, "Solltemp.: %d.%d C", s_int, s_frac);
      snprintf(content[2], 21, "Zustand:   %s", stringOfStates[0]);
      snprintf(content[3], 21, "Mode:      %s", stringOfStates[1]);
      break;
    }

    case OutputIntent::LCD_Display_state::OFF: {
      lcd.noBacklight();
      lcd.noDisplay();
      break;
    }
    }
  }

  void init() {
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();
  }

  void update_wrapper(OutputIntent::LCD_Display_state state,
                      Display_content content) {
    statesToStrings(content.heatingstate, content.mode);
  }
};

class InputDevices {
private:
  friend class SystemController;
  ToggleSwitchDriver m_powerSwitch{2};
  ToggleSwitchDriver m_modeSwitch{3};
  PushButtonDriver m_displayButton{8};
  MyEncoderDriver m_myEncoder{6, 7};
  TemperatureSensorDriver m_DS18B20{5};

  InputDevices() = default;

  void init() {
    m_powerSwitch.init();
    m_modeSwitch.init();
    m_displayButton.init();
    m_myEncoder.init();
    m_DS18B20.init();
  }

  void pollingDevicesAndUpdateData(InputData &output) {
    output.powerSwitchHasChanged = m_powerSwitch.hasChanged();
    output.modeSwitchHasChanged = m_modeSwitch.hasChanged();
    output.displayButtonHasChanged = m_displayButton.isPressed();
    output.encoderVal = m_myEncoder.updateVal();
    output.DS18B20_tempC = m_DS18B20.updateTemp();
  }
};

class OutputDevices {
private:
  friend class SystemController;
  RelaisDriver relais{4};
  OutputIntent &m_outputintent;
  DisplayDriver lcdDisplay; // Keine PIN Zuweisung nötig, da Arduino I2C Bus
                            // automatisch erkennt.
  OutputDevices(OutputIntent &oi)
      : m_outputintent(oi), lcdDisplay(m_outputintent.display_content,
                                       m_outputintent.lcd_display_state) {}

  void init() {
    relais.init();
    lcdDisplay.init();
  }

  void update() {
    relais.update(m_outputintent.relais_intent);
    resetHandledOutputIntent();
  }

  void resetHandledOutputIntent() {
    m_outputintent.relais_intent = OutputIntent::RelaisIntent::neutral;
  }
};

class SystemController {
public:
  InputDevices inputdevices;
  InputData inputdata;
  HeaterStatus heaterStatus;
  OutputIntent outputintent;
  OutputDevices outputdevices{outputintent};

  SystemController() = default;

  void operator()() {
    inputdevices.pollingDevicesAndUpdateData(inputdata);
    applyInputdata();
    applyHeatingLogic();
    writeOutputIntent();
    outputdevices.update();
  }

  void init() {
    heaterStatus.heatingstate = HeaterStatus::HeatingState::OFF;
    heaterStatus.mode = HeaterStatus::Mode::TEMP;
    delay(500); // Damit sich Hardware kurz einpendeln kann
    inputdevices.init();
    outputdevices.init();
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
    if (inputdata.powerSwitchHasChanged) {
      if (heaterStatus.heatingstate == HeaterStatus::HeatingState::ON) {
        outputintent.relais_intent = OutputIntent::RelaisIntent::long_;
        heaterStatus.heatingstate = HeaterStatus::HeatingState::OFF;
        heaterStatus.mode = HeaterStatus::Mode::POWER;

      } else
        outputintent.relais_intent = OutputIntent::RelaisIntent::long_;
      heaterStatus.heatingstate = HeaterStatus::HeatingState::ON;
      heaterStatus.mode = HeaterStatus::Mode::POWER;
    }
    // Mode Switch Logik
    if (inputdata.modeSwitchHasChanged) {
      if (heaterStatus.mode == HeaterStatus::Mode::POWER) {
        outputintent.relais_intent = OutputIntent::RelaisIntent::short_;
        heaterStatus.mode = HeaterStatus::Mode::TEMP;
      } else
        outputintent.relais_intent = OutputIntent::RelaisIntent::short_;
      heaterStatus.mode = HeaterStatus::Mode::POWER;
    }

    // Encoder Logik
    //  Hier wird der Modus geprüft, weil der Drehregler mit Knopf vllt mal die
    //  Ansicht auf Laufzeitdaten wechseln soll.
    if (inputdata.encoderVal != 0) {
      if (heaterStatus.mode == HeaterStatus::Mode::TEMP) {
        heaterStatus.Solltemperatur +=
            inputdata.encoderVal * TempStep; // encoderVal hat ist signed
        if (heaterStatus.Solltemperatur > TempMax)
          heaterStatus.Solltemperatur = TempMax;
        else if (heaterStatus.Solltemperatur < TempMin)
          heaterStatus.Solltemperatur = TempMin;
      }
    }
    if (inputdata.displayButtonHasChanged) {
      if (outputintent.lcd_display_state == OutputIntent::LCD_Display_state::ON)
        outputintent.lcd_display_state = OutputIntent::LCD_Display_state::OFF;
      else if (outputintent.lcd_display_state ==
               OutputIntent::LCD_Display_state::OFF)
        outputintent.lcd_display_state = OutputIntent::LCD_Display_state::ON;
    }
  }

  // Temperaturregelung
  void applyHeatingLogic() {
    constexpr float Solltoleranz = 1.5;

    if (heaterStatus.mode == HeaterStatus::Mode::TEMP) {
      if (inputdata.DS18B20_tempC <
              heaterStatus.Solltemperatur - Solltoleranz &&
          heaterStatus.heatingstate == HeaterStatus::HeatingState::OFF) {
        outputintent.relais_intent = OutputIntent::RelaisIntent::long_;
        heaterStatus.heatingstate = HeaterStatus::HeatingState::ON;
      } else if (inputdata.DS18B20_tempC >
                     heaterStatus.Solltemperatur + Solltoleranz &&
                 heaterStatus.heatingstate == HeaterStatus::HeatingState::ON) {
        outputintent.relais_intent == OutputIntent::RelaisIntent::long_;
        heaterStatus.heatingstate == HeaterStatus::HeatingState::OFF;
      }
    } else
      return;
  }

  void writeOutputIntent() {
    outputintent.display_content.tempC = inputdata.DS18B20_tempC;
    outputintent.display_content.Solltemperatur = heaterStatus.Solltemperatur;
    outputintent.display_content.heatingstate = heaterStatus.heatingstate;
    outputintent.display_content.mode = heaterStatus.mode;
  }
};

/*Dann in main.ino bzw main.cpp
SystemController controller;

void setup() { controller.init(); };

void loop() { controller(); }*/
