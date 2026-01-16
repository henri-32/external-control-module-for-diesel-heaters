// variables.h
#pragma once
#include <DallasTemperature.h>
#include <Encoder.h>
#include <OneWire.h>

enum class HEIZUNGSZUSTAND : uint8_t { AUS, AN };

enum class HEIZUNGSMODE : uint8_t { TEMP, POWER };

enum class RELAISZUSTAND : uint8_t { AUS, AN500, AN2000 };

enum class HEIZUNG_STARTBEREIT : uint8_t { nichtbereit, bereit };

enum class DEBUGMODE : uint8_t { running, debug, oledprint };

enum class RAUMTEMPERATUR : uint8_t { richtig, kalt, warm, Fehler };

extern OneWire oneWire;
extern DallasTemperature sensors;

extern HEIZUNGSZUSTAND Heizungszustand;
extern HEIZUNGSMODE Heizungsmode;
extern RELAISZUSTAND Relaiszustand;
extern DEBUGMODE debugmode;
extern RAUMTEMPERATUR Raumtemperatur;

// Pin Definitionen
extern const int OnOffSwitchPin;
extern const int modeSwitchPin;
extern const int ds18b20Pin;
extern const int outputRelaisPin;
extern const int TRAPin;
extern const int TRBPin;
extern const int encoderswPin;

extern bool tempRequestPending;

extern float tempC;
extern float Solltemperatur;
extern const float Toleranz_Solltemperatur;
extern Encoder encoder;

extern unsigned long lastDebounceTimeanaus;
extern unsigned long lastDebounceTimemode;
extern const unsigned long debounceDelay;

extern unsigned long lastdebugprint;
extern const unsigned long debugprintintervall;

extern unsigned long lastrelais2000time;
extern unsigned long lastrelais500time;
extern const unsigned long relais2000delay;
extern const unsigned long relais500delay;

extern unsigned long beginnsperretimer;
extern const unsigned long dauersperre;

extern unsigned long lastTempRequest;
extern const unsigned long tempMeasurementDelay;
