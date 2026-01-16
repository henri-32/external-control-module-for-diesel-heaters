#include "classes.h"
#include "Arduino.h"
#include "variables.h"

// Zu Testzwecken hier definiert
const int OnOffSwitchPin = 2;
int test;
// Klasseninstanzen

class Switches {
public:
  explicit Switches(uint8_t pin)
      : pin(pin), switchValue(false), lastSwitchValue(false) {}
  void init() {
    pinMode(pin, INPUT_PULLUP);
    switchValue = digitalRead(pin);
    lastSwitchValue = switchValue;
  }

  bool hasSwitched() {
    switchValue = digitalRead(pin);
    if (switchValue == lastSwitchValue)
      return false;
    lastSwitchValue = switchValue;
    return true;
  }

private:
  uint8_t pin;
  bool switchValue;
  bool lastSwitchValue;
};

Switches onOffSwitch{OnOffSwitchPin};

void testfunction() {
  if (onOffSwitch.hasSwitched() == true)
    test = 1;
}