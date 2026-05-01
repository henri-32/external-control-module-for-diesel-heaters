#include "ArduinoStubs.h"
using namespace ArduinoStub::Spies;

int ArduinoStub::Spies::pinWritten_state;
PinState ArduinoStub::Spies::writtenState;
int ArduinoStub::Spies::pinWritten_mode;
PinMode ArduinoStub::Spies::writtenMode;
static unsigned long fake_now_ms = 0;

void digitalWrite(int pin, PinState state) {
  pinWritten_state = pin;
  writtenState = state;
};

void pinMode(int pin, PinMode mode) {
  pinWritten_mode = pin;
  writtenMode = mode;
};

unsigned long millis() {
  return fake_now_ms;
}

void setMillis(unsigned long now_ms) { fake_now_ms = now_ms; }

void advanceMillis(unsigned long delta_ms) { fake_now_ms += delta_ms; }
