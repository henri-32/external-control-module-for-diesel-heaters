#pragma once

// Architekturentscheidung:
// Diese Datei stellt im TEST_BUILD minimale Arduino-Stubs mit den originalen
// globalen Funktionssignaturen bereit (z. B. digitalWrite/pinMode), damit der
// Produktivcode unverändert testbar bleibt. Die zugehörigen Spy-Variablen
// liegen getrennt im Namespace und werden ausschließlich von Unit-Tests zur
// Verifikation von Arduino-Aufrufen ausgewertet.


// DESHALB MÜSSEN BEI UNIT TESTS SETUPS() MIT PASSENDEN INITIALWERTEN GENUTZT WERDEN 

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"

enum PinState { HIGH, LOW };
enum PinMode { OUTPUT, INPUT };

extern void digitalWrite(int pin, PinState state);
extern void pinMode(int pin, PinMode mode);
extern unsigned long millis();
extern void setMillis(unsigned long now_ms);
extern void advanceMillis(unsigned long delta_ms);

namespace ArduinoStub::Spies {

extern int pinWritten_state;
extern PinState writtenState;

extern int pinWritten_mode;
extern PinMode writtenMode;
};

#pragma GCC diagnostic pop
