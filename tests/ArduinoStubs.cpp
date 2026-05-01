#include "ArduinoStubs.h"
using namespace ArduinoStub::Spies;

int ArduinoStub::Spies::pin_set_to;
PinState ArduinoStub::Spies::state_set_to;
int ArduinoStub::Spies::mode_of_pin_set;
PinMode ArduinoStub::Spies::mode_set_to;

void digitalWrite(int pin, PinState state) {
  pin_set_to = pin;
  state_set_to = state;
};

void pinMode(int pin, PinMode mode) {
  mode_of_pin_set = pin;
  mode_set_to = mode;
};

unsigned long millis() {
  static unsigned long now = 1000;
  now += 1000;
  return now;
}
