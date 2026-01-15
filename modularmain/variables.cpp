// variables.cpp
#include "variables.h"

// --------- Ein- und Ausgänge---------
const int OnOffSwitchPin = 2;
const int modeSwitchPin = 3;
const int ds18b20Pin = 5;
const int outputRelaisPin = 4;
const int TRAPin = 6;
const int TRBPin = 7;
const int encoderswPin = 8;

//--------- Bools --------------
bool tempRequestPending =
    false; // Temperaturmessung angefordert Ergebnis ausstehend

// Variablen
float tempC;
float Solltemperatur = 20.0;
const float Toleranz_Solltemperatur = 1.5;
Encoder encoder(TRAPin, TRBPin);

//--------- Zeitvariablen -----------
unsigned long lastDebounceTimeanaus = 0;
unsigned long lastDebounceTimemode = 0;
const unsigned long debounceDelay = 50;

unsigned long lastdebugprint = 0;
const unsigned long debugprintintervall = 1500;

unsigned long lastrelais2000time = 0;
unsigned long lastrelais500time = 0;
const unsigned long relais2000delay = 2000;
const unsigned long relais500delay = 500;

unsigned long beginnsperretimer = 0;
const unsigned long dauersperre = 360000;

unsigned long lastTempRequest = 0;
const unsigned long tempMeasurementDelay = 750; // für 12-Bit DS18B20

// ------------ Zustansvariablen (enum classes) --------------

HEIZUNGSZUSTAND Heizungszustand = HEIZUNGSZUSTAND::AUS;
HEIZUNGSMODE Heizungsmode = HEIZUNGSMODE::TEMP;
RELAISZUSTAND Relaiszustand = RELAISZUSTAND::AUS;
HEIZUNG_STARTBEREIT Heizung_startbereit = HEIZUNG_STARTBEREIT::nichtbereit;
RAUMTEMPERATUR Raumtemperatur = RAUMTEMPERATUR::richtig;
ART_DES_SCHALTENS Art_des_Schaltens = ART_DES_SCHALTENS::nichtgesetzt;
TEMPERATURSPERRE Temperatursperre = TEMPERATURSPERRE::nichtaktiv;
DEBUGMODE debugmode = DEBUGMODE::debug;