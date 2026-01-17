#include "classes.h"
#include "Arduino.h"
#include <Encoder.h>

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

private:
  Encoder encoder;
  long value;
  long lastValue = 0;
  int diff = value - lastValue;
  int delta;
  long lastAction = 0;
};

class Inputs {

public:
  Inputs() : onOffSwitch(2), modeSwitch(3), displaySwitch(8){};

  void init() {
    onOffSwitch.init();
    modeSwitch.init();
    displaySwitch.init();
  }

  void poll() {
    onOffSwitch.hasChanged();
    modeSwitch.hasChanged();
    displaySwitch.isPressed();
  }

private:
  ToggleSwitches onOffSwitch;
  ToggleSwitches modeSwitch;
  PushButton displaySwitch;
};
Inputs inputs;

void testsetup() { inputs.init(); };
void testloop() { inputs.poll(); };