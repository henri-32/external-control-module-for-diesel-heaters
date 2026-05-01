#include "ArduinoStubs.h"
using namespace ArduinoStub::Spies;

int ArduinoStub::Spies::pinWritten_state;
PinState ArduinoStub::Spies::writtenState;
int ArduinoStub::Spies::pinWritten_mode;
PinMode ArduinoStub::Spies::writtenMode;

void digitalWrite(int pin, PinState state) {
  pinWritten_state = pin;
  writtenState = state;
};

void pinMode(int pin, PinMode mode) {
  pinWritten_mode = pin;
  writtenMode = mode;
};

unsigned long millis() {
  static unsigned long now = 1000;
  now += 1000;
  return now;
}
