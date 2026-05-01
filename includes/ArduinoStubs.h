#pragma once

// Architekturentscheidung:
// Diese Datei stellt im TEST_BUILD minimale Arduino-Stubs mit den originalen
// globalen Funktionssignaturen bereit (z. B. digitalWrite/pinMode), damit der
// Produktivcode unverändert testbar bleibt. Die zugehörigen Spy-Variablen
// liegen getrennt im Namespace und werden ausschließlich von Unit-Tests zur
// Verifikation von Arduino-Aufrufen ausgewertet.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"

enum PinState { HIGH, LOW };
enum PinMode { OUTPUT, INPUT };

extern void digitalWrite(int pin, PinState state);
extern void pinMode(int pin, PinMode mode);
extern unsigned long millis();

namespace ArduinoStub::Spies {

extern int pin_set_to;
extern PinState state_set_to;

extern int mode_of_pin_set;
extern PinMode mode_set_to;
};

#pragma GCC diagnostic pop
