#include "classes.h"
#include "Arduino.h"
#include <DallasTemperature.h>
#include <Encoder.h>
#include <OneWire.h>
#include <cstdint>

// Forward declarations
struct InputDataSTRUCT {
  bool onOffSwitchHasChanged;
  bool modeSwitchHasChanged;
  bool displayButtonHasChanged;
  int8_t myEncoderHasChanged;
  int8_t DS18B20_tempC;
};

// Klasseninstanzen Unterklassen von HardwareInputs

class ToggleSwitches {
public:
  explicit ToggleSwitches(uint8_t pin) : m_pin(pin){};

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
  long m_lastDebounce = 0;
  const long m_debounceDelay = 50;
};

class PushButtons {
public:
  explicit PushButtons(uint8_t pin) : m_pin(pin) {}
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
    return true;
  }

private:
  const uint8_t m_pin;
  bool m_buttonVal;
  bool m_lastButtonVal;
  bool m_lastDebounce = 0;
  const bool m_debounceDelay = 50;
};

class MyEncoders { // Name gewählt, da Bibliothek Encoder heißt.
public:
  explicit MyEncoders(uint8_t pin1, uint8_t pin2) : m_encoder(pin1, pin2){};

private:
  void poll() {
    m_value = m_encoder.read();
    if (m_value == m_lastValue)
      return;
    m_delta += m_diff;
    m_lastAction = millis();
    m_lastValue = m_value;
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
  int update() { // So kann nur ein Aufruf erfolgen, allerdings bleibt die
                 // Refreshrate der beiden Funktionen unterschiedlich
    poll();
    return translateStepsToInput();
  }

private:
  Encoder m_encoder; // Encoder library genutzt
  long m_value;
  long m_lastValue = 0;
  int m_diff = m_value - m_lastValue;
  int m_delta;
  long m_lastAction = 0;
};

class TenperatureSensors {
public:
  explicit TenperatureSensors(uint8_t pin)
      : m_oneWire(pin), m_sensors(&m_oneWire) {}

  void init() {
    m_sensors.begin();
    m_sensors.requestTemperatures();
    m_tempC = m_sensors.getTempCByIndex(0);
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

public:
  float update() {
    startTemperatureRequest();
    measureTemperature();
    return m_tempC;
  }

private:
  OneWire m_oneWire;
  DallasTemperature m_sensors;
  float m_tempC;
  unsigned long m_lastTempRequest = 0;
  bool m_tempRequestPending = false;
  static constexpr unsigned long m_REQUEST_INTERVAL_MS = 2000;
  // Static heißt hier alle Objekte nutzen gleiche Variable
  static constexpr unsigned long m_CONVERSION_TIME_MS = 750;
};

class AllInputDevices {
public:
  AllInputDevices()
      : m_onOffSwitch(2), m_modeSwitch(3), m_displayButton(8),
        m_myEncoder(6, 7), m_DS18B20(5){};

  void init() {
    m_onOffSwitch.init();
    m_modeSwitch.init();
    m_displayButton.init();
    m_DS18B20.init();
  }

  void pollingAndUpdate(InputDataSTRUCT &output) {
    output.onOffSwitchHasChanged = m_onOffSwitch.hasChanged();
    output.modeSwitchHasChanged = m_modeSwitch.hasChanged();
    output.displayButtonHasChanged = m_displayButton.isPressed();
    output.myEncoderHasChanged = m_myEncoder.update();
    output.DS18B20_tempC = m_DS18B20.update();
  }

private:
  ToggleSwitches m_onOffSwitch;
  ToggleSwitches m_modeSwitch;
  PushButtons m_displayButton;
  MyEncoders m_myEncoder;
  TenperatureSensors m_DS18B20;
};

struct HeaterStatesSTRUCT {
  enum class HEIZUNGSZUSTAND { ON, OFF };
  HEIZUNGSZUSTAND Heizungszustand = HEIZUNGSZUSTAND::OFF;

  enum class HEIZUNGSMODE { TEMP, POWER };
  HEIZUNGSMODE Heizungsmode = HEIZUNGSMODE::TEMP;

  enum class RAUMTEMPERATUR { neutral, kalt, warm };
  RAUMTEMPERATUR Raumtemperatur = RAUMTEMPERATUR::neutral;
};

class Relais {
public:
  explicit Relais(const uint8_t pin) : m_pin(pin) {}

  void init() {
    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, LOW);
  }

  void request(uint16_t duration) {
    if (m_relaisState == RelaisState::ON)
      return;
    m_duration = duration;
    activate();
  }

  void update() {
    if (m_relaisState == RelaisState::OFF)
      return;

    if (millis() - m_lastRelaisActivation < m_duration)
      return;
    deactivate();
  }

private:
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
  uint16_t m_duration = 0;
};

AllInputDevices hardwareinputs;
Relais relais(4); // Frage an ChatGPT Darf das in main.ino instanziert werden,
                  // wenn main.ino diese Datei inkludiert? Bzw. konkreter,
                  // wenn Dateien inkludiert werden, wie ist die
                  // Aufrufreihenfolge?

                  
class ControllerCLASS {
public:
  explicit ControllerCLASS() : inputData(), heaterStates() {}

  void initAllHardware() {
    hardwareinputs.init();
    relais.init();
  }

  void Runtime() {
    hardwareinputs.pollingAndUpdate(inputData);
    relais.update();
  }

private:
  InputDataSTRUCT inputData;
  HeaterStatesSTRUCT heaterStates;
};

// Dann in main.ino

ControllerCLASS Controller;

void setup() { Controller.initAllHardware(); };

void loop() { Controller.Runtime(); }