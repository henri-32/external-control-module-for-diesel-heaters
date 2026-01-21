#include "classes.h"
#include "Arduino.h"
#include <DallasTemperature.h>
#include <Encoder.h>
#include <OneWire.h>
#include <LiquidCrystal_I2C.h>
#include <cstdint>

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
  HeatingState heatingstate = HeatingState::OFF;

  enum class Mode { TEMP, POWER };
  Mode mode = Mode::TEMP;
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

  void init(){
    poll();
  }

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
  explicit RelaisDriver(const uint8_t pin) : m_pin(pin) {}
  enum class RelaisDuration { long_, short_, init };
  RelaisDuration m_relais_duration;
  uint16_t m_relais_duration_int = 0;

public:
  void init() {
    pinMode(m_pin, OUTPUT);
    m_relais_duration = RelaisDuration::init;
    digitalWrite(m_pin, LOW);
  }

  void request(RelaisDriver::RelaisDuration relais_duration) {
    if (m_relaisState == RelaisState::ON)
      return;
    m_relais_duration = relais_duration;
    activate();
  }

  void update() {
    convertRelaisDurationToInt();
    if (m_relaisState == RelaisState::OFF)
      return;

    if (millis() - m_lastRelaisActivation < m_relais_duration_int)
      return;
    deactivate();
  }

private:
  void convertRelaisDurationToInt() {
    if (m_relais_duration == RelaisDuration::long_)
      m_relais_duration_int = 2000;
    else if (m_relais_duration == RelaisDuration::short_)
      m_relais_duration_int = 500;
    else if (m_relais_duration == RelaisDuration::init)
      m_relais_duration_int = 0;
  }
  void activate() {
    digitalWrite(m_pin, HIGH);
    m_lastRelaisActivation = millis();
    m_relaisState = RelaisState::ON;
  }

  void deactivate() {
    digitalWrite(m_pin, LOW);
    m_relaisState = RelaisState::OFF;
  }

  const uint8_t m_pin;
  unsigned long m_lastRelaisActivation = 0;
  enum class RelaisState { ON, OFF };
  RelaisState m_relaisState = RelaisState::OFF;
};

class LCDDisplay  {
private:
friend class SystemController;
  LCDDisplay() : content[][] (), stringOfStates[][]() {}
  LiquidCrystal_I2C lcd (0x27, 20, 4);

    static void clearLine(uint8_t line){
      lcd.setCursor(0, line);
      lcd.print("                    ");
      lcd.setCursor(0,line);
    }

    void init(){
      Wire.begin();
      lcd.init();
      lcd.backlight();
      lcd.clear();
    }

    char content[4][21]; 
    char stringOfStates[4][21];
  


};

class InputDevices {
private:
  friend class SystemController;
  InputDevices()
      : m_powerSwitch(2), m_modeSwitch(3), m_displayButton(8),
        m_myEncoder(6, 7), m_DS18B20(5){};

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

private:
  ToggleSwitchDriver m_powerSwitch;
  ToggleSwitchDriver m_modeSwitch;
  PushButtonDriver m_displayButton;
  MyEncoderDriver m_myEncoder;
  TemperatureSensorDriver m_DS18B20;
};

class OutputDevices {
private:
  friend class SystemController;
  OutputDevices() : relais(4) {}

  void init() { relais.init(); }

  void update() { relais.update(); }
  RelaisDriver relais;
};

class SystemController {
public:
  explicit SystemController() : heaterStatus(), inputdata() {}

  void operator()() {
    inputdevices.pollingDevicesAndUpdateData(inputdata);
    applyInputdata();
    applyHeatingLogic();
    outputdevices.update();
  }

  void init() {
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

    if (inputdata.powerSwitchHasChanged) {
      if (heaterStatus.heatingstate == HeaterStatus::HeatingState::ON) {
        outputdevices.relais.request(RelaisDriver::RelaisDuration::long_);
        heaterStatus.heatingstate = HeaterStatus::HeatingState::OFF;
        heaterStatus.mode = HeaterStatus::Mode::POWER;

      } else
        outputdevices.relais.request(RelaisDriver::RelaisDuration::long_);
      heaterStatus.heatingstate = HeaterStatus::HeatingState::ON;
      heaterStatus.mode = HeaterStatus::Mode::POWER;
    }

    if (inputdata.modeSwitchHasChanged) {
      if (heaterStatus.mode == HeaterStatus::Mode::POWER) {
        outputdevices.relais.request(RelaisDriver::RelaisDuration::short_);
        heaterStatus.mode = HeaterStatus::Mode::TEMP;
      } else
        outputdevices.relais.request(RelaisDriver::RelaisDuration::short_);
      heaterStatus.mode = HeaterStatus::Mode::POWER;
    }

    // Hier wird der Modus geprüft, weil der Drehregler mit Knopf vllt mal die
    // Ansicht auf Laufzeitdaten wechseln soll.
    if (inputdata.encoderVal != 0) {
      if (heaterStatus.mode == HeaterStatus::Mode::TEMP) {
        Solltemperatur +=
            inputdata.encoderVal * TempStep; // encoderVal hat ist signed
        if (Solltemperatur > TempMax)
          Solltemperatur = TempMax;
        else if (Solltemperatur < TempMin)
          Solltemperatur = TempMin;
      }
    }
  }

  void applyHeatingLogic() {
    constexpr float Solltoleranz = 1.5;

    if (heaterStatus.mode == HeaterStatus::Mode::TEMP) {
      if (inputdata.DS18B20_tempC < Solltemperatur - Solltoleranz &&
          heaterStatus.heatingstate == HeaterStatus::HeatingState::OFF) {
        outputdevices.relais.request(RelaisDriver::RelaisDuration::long_);
        heaterStatus.heatingstate = HeaterStatus::HeatingState::ON;
      } else if (inputdata.DS18B20_tempC > Solltemperatur + Solltoleranz &&
                 heaterStatus.heatingstate == HeaterStatus::HeatingState::ON) {
        outputdevices.relais.request(RelaisDriver::RelaisDuration::long_);
        heaterStatus.heatingstate == HeaterStatus::HeatingState::OFF;
      }
    } else
      return;
  }

  HeaterStatus heaterStatus;
  InputData inputdata;
  float Solltemperatur = 20;
  uint8_t pin;
  InputDevices inputdevices;
  OutputDevices outputdevices;
};

// Dann in main.ino bzw main.cpp

SystemController controller;

void setup() { controller.init(); };

void loop() { controller(); }

