#include "ArduinoStubs.h"
using namespace ArduinoStubSpies;

// Diese Funktionen müssen global sein, umd er Arduino Signatur zu entsprechen
void digitalWrite(int pin, PinState state) {
  pinWritten_state = pin;
  writtenState = state;
};

void pinMode(int pin, PinMode mode) {
  pinWritten_mode = pin;
  writtenMode = mode;
};

static unsigned long fake_now_ms = 0;
unsigned long millis() {
  return fake_now_ms;
}

//Eigene Hilfsfunktionen zum Testen der Stubs
int ArduinoStubSpies::pinWritten_state;
PinState ArduinoStubSpies::writtenState;
int ArduinoStubSpies::pinWritten_mode;
PinMode ArduinoStubSpies::writtenMode;

void ArduinoStubSpies::setMillis(unsigned long now_ms) { fake_now_ms = now_ms; }

void ArduinoStubSpies::advanceMillis(unsigned long delta_ms) { fake_now_ms += delta_ms; }

void ArduinoStubSpies::resetSpies(){
 pinWritten_state = -1;  
 writtenState = NOSTATE;
 pinWritten_mode = -1; 
 writtenMode = NOMODE;
};
