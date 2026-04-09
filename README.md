## Projekt
main
- Aktuelles Hauptprogramm

LCD Display
- Implementierung des Displays

Experimente
- Code Schnipsel etc.

## CMake / compile_commands.json

### Schneller Start (statische Analyse, ohne Arduino-SDK)
```bash
cmake -S . -B build
cmake --build build
```

Die Datei `compile_commands.json` wird unter `build/compile_commands.json` erzeugt.

Optional für clangd im Repo-Root:
```bash
ln -sf build/compile_commands.json compile_commands.json
```

### Mit echten Arduino-Headern
Wenn du statt der Stub-Header die echten Arduino-Pfade nutzen willst:
```bash
cmake -S . -B build \
  -DHEIZUNG_USE_ARDUINO_SDK=ON \
  -DARDUINO_CORE_DIR=/pfad/zu/cores/arduino \
  -DARDUINO_VARIANT_DIR=/pfad/zu/variants/standard \
  -DARDUINO_WIRE_DIR=/pfad/zu/libraries/Wire/src \
  -DARDUINO_LIB_ENCODER_DIR=/pfad/zu/Encoder \
  -DARDUINO_LIB_DALLAS_DIR=/pfad/zu/DallasTemperature \
  -DARDUINO_LIB_ONEWIRE_DIR=/pfad/zu/OneWire \
  -DARDUINO_LIB_LCD_I2C_DIR=/pfad/zu/LiquidCrystal_I2C
```

### Unity-Test-Einstieg (optional)
Der Einstiegstest liegt in `tests/test_unity_entry.cpp`.
Die zentrale Testliste liegt in `tests/test_cases.def`.

#### Setup
Unity einbinden (z. B. als `third_party/unity` mit `src/unity.c` und `src/unity.h`) und dann:
```bash
cmake -S . -B build-tests -DHEIZUNG_ENABLE_TESTS=ON -DUNITY_ROOT=third_party/unity
cmake --build build-tests -j
```

Tests ausführen:
```bash
ctest --test-dir build-tests --output-on-failure
```

#### Wie der Runner arbeitet
- `tests/test_unity_entry.cpp` enthält die Testfunktionen und den Unity-`main(...)`.
- `tests/test_cases.def` ist die zentrale Liste aller Testfunktionen.
- Dieselbe Liste wird zweimal verwendet:
- im C++-Runner für `RUN_TEST(...)`
- in CMake für `add_test(...)`

Dadurch entsteht pro Testfunktion ein eigener CTest-Testfall (z. B. `unity.test_applyModeSwitchInput`), obwohl nur ein Test-Binary gebaut wird.

#### Neue Tests hinzufügen
1. Testfunktion in `tests/test_unity_entry.cpp` anlegen.
2. Funktionsnamen in `tests/test_cases.def` eintragen:
```c
TEST_CASE(test_mein_neuer_fall)
```
3. Neu konfigurieren/bauen und Tests laufen lassen:
```bash
cmake -S . -B build-tests -DHEIZUNG_ENABLE_TESTS=ON -DUNITY_ROOT=third_party/unity
cmake --build build-tests -j
ctest --test-dir build-tests --output-on-failure
```
