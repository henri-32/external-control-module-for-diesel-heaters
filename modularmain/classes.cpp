#include "classes.h"
#include "Arduino.h"
#include <DallasTemperature.h>
#include <Encoder.h>
#include <OneWire.h>

// Klasseninstanzen

class ToggleSwitches {
public:
  explicit ToggleSwitches(uint8_t pin) : pin(pin){};

  void init() {
    pinMode(pin, INPUT_PULLUP);
    switchVal = digitalRead(pin);
    lastSwitchVal = switchVal;
  };

  bool hasChanged() {
    switchVal = digitalRead(pin);
    if (switchVal == lastSwitchVal)
      return false;
    lastSwitchVal = switchVal;
    return true;
  };

private:
  const uint8_t pin;
  bool switchVal;
  bool lastSwitchVal;
};

class PushButton {
public:
  explicit PushButton(uint8_t pin) : pin(pin) {}
  void init() {
    pinMode(pin, INPUT_PULLUP);
    buttonVal = digitalRead(pin);
    lastButtonVal = buttonVal;
  }

  bool isPressed() {
    buttonVal = digitalRead(pin);
    if (buttonVal == lastButtonVal)
      return false;
    if (buttonVal == HIGH)
      return false; // Hier wird im Gegensatz zum ToggleSwitch nur auf das
                    // Drücken reagiert, nicht aufs Loslassen
    return true;
  }

private:
  const uint8_t pin;
  bool buttonVal;
  bool lastButtonVal;
};

class MyEncoders { // Name gewählt, da Bibliothek Encoder heißt.
public:
  explicit MyEncoders(uint8_t pin1, uint8_t pin2) : encoder(pin1, pin2){};

private:
  void poll() {
    value = encoder.read();
    if (value == lastValue)
      return;
    delta += diff;
    lastAction = millis();
    lastValue = value;
  }

  int translateStepsToInput() {
    if (millis() - lastAction < 100)
      return 0;
    int steps = delta / 4; // Wird auf ganze Schritte runtergebrochen und rest
                           // vernichtet bei Ganzzahldivision
    delta -= steps * 4;    // Das Delta wird um die Schritte die ganz gemacht
                        // wurden wieder reduziert (in Rohwerten) so bleibt der
                        // Rest für den nächsten aufruf erhalten
    return steps;
  };

public:
  int update() { // So kann nur ein Aufruf erfolgen, allerdings bleibt die
                 // Refreshrate der beiden Funktionen unterschiedlich
    poll();
    return translateStepsToInput();
  }

private:
  Encoder encoder;
  long value;
  long lastValue = 0;
  int diff = value - lastValue;
  int delta;
  long lastAction = 0;
};

class TemperatureSensors {
public:
  explicit TemperatureSensors(uint8_t pin) : oneWire(pin), sensors(&oneWire) {}

  void init() {
    sensors.begin();
    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(0);
  }

private:
  void startTemperatureRequest() {
    if (millis() - lastTempRequest < REQUEST_INTERVAL_MS)
      return;
    if (tempRequestPending)
      return;
    sensors.requestTemperatures();
    lastTempRequest = millis();
    tempRequestPending = true;
  }

  void measureTemperature() {
    if (!tempRequestPending)
      return;
    if (millis() - lastTempRequest < CONVERSION_TIME_MS)
      return;
    // Zeit die Messung DS18B20 braucht
    tempC = sensors.getTempCByIndex(0);
    tempRequestPending = false;
  };

public:
  float update() {
    startTemperatureRequest();
    measureTemperature();
    return tempC;
  }

private:
  OneWire oneWire;
  DallasTemperature sensors;
  float tempC;
  unsigned long lastTempRequest = 0;
  bool tempRequestPending = false;
  static constexpr unsigned long REQUEST_INTERVAL_MS = 2000;
  // Static heißt hier alle Objekte nutzen gleiche Variable
  static constexpr unsigned long CONVERSION_TIME_MS = 750;
};

class Inputs {

public:
  Inputs()
      : onOffSwitch(2), modeSwitch(3), displayButton(8), myEncoder(6, 7),
        DS18B20(5){};

  void init() {
    onOffSwitch.init();
    modeSwitch.init();
    displayButton.init();
    DS18B20.init();
  }

  void poll() {
    onOffSwitch.hasChanged();
    modeSwitch.hasChanged();
    displayButton.isPressed();
    myEncoder.update();
    DS18B20.update();
  }

private:
  ToggleSwitches onOffSwitch;
  ToggleSwitches modeSwitch;
  PushButton displayButton;
  MyEncoders myEncoder;
  TemperatureSensors DS18B20;
};
Inputs inputs;

void testsetup() { inputs.init(); };
void testloop() { inputs.poll(); };