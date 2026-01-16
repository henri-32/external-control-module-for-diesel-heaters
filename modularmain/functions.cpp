#include "functions.h"
#include "lcddisplay.h"
#include "variables.h"
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

// Bibliotheken verknüpfen
OneWire oneWire(ds18b20Pin);
DallasTemperature sensors(&oneWire);

void pinSettings() {
  pinMode(OnOffSwitchPin, INPUT_PULLUP);
  pinMode(modeSwitchPin, INPUT_PULLUP);
  pinMode(ds18b20Pin, INPUT); // Weil externer Pullup
  pinMode(outputRelaisPin, OUTPUT);
  pinMode(TRAPin, INPUT_PULLUP);
  pinMode(TRBPin, INPUT_PULLUP);
  pinMode(encoderswPin, INPUT_PULLUP);
  digitalWrite(outputRelaisPin, LOW);
}

void startRelais(RELAISZUSTAND zielzustand, unsigned long now) {
  if (zielzustand == RELAISZUSTAND::AN2000) {
    if (now - lastDebounceTimeanaus < debounceDelay)
      return;
    digitalWrite(outputRelaisPin, HIGH);
    lastDebounceTimeanaus = now;
    lastrelais2000time = now;
    Relaiszustand = zielzustand;
    Heizungszustand == HEIZUNGSZUSTAND::AUS
        ? Heizungszustand = HEIZUNGSZUSTAND::AN
        : Heizungszustand = HEIZUNGSZUSTAND::AUS;
    if (debugmode == DEBUGMODE::debug)
      Serial.println("Relaistart Zielzustand AN2000");
  } else if (zielzustand == RELAISZUSTAND::AN500) {
    if (now - lastDebounceTimemode < debounceDelay)
      return;
    digitalWrite(outputRelaisPin, HIGH);
    lastDebounceTimemode = now;
    lastrelais500time = now;
    Relaiszustand = zielzustand;
    Heizungsmode = (Heizungsmode == HEIZUNGSMODE::TEMP) ? HEIZUNGSMODE::POWER
                                                        : HEIZUNGSMODE::TEMP;
    if (debugmode == DEBUGMODE::debug)
      Serial.println("Relaistart Zielzustand AN500");
  }
}

void relaischeck_loesen(unsigned long now) {
  if (Relaiszustand == RELAISZUSTAND::AUS)
    return;

  if (Relaiszustand == RELAISZUSTAND::AN500 &&
      now - lastrelais500time > relais500delay) {
    digitalWrite(outputRelaisPin, LOW);
    Relaiszustand = RELAISZUSTAND::AUS;
  }

  if (Relaiszustand == RELAISZUSTAND::AN2000 &&
      now - lastrelais2000time > relais2000delay) {
    digitalWrite(outputRelaisPin, LOW);
    Relaiszustand = RELAISZUSTAND::AUS;
    if (debugmode == DEBUGMODE::debug)
      Serial.println("Relais gelöst");
  }
}

// Beginn aller Schalterfunktionen
//
void readOnOfSwitch(unsigned long now, bool initComplete) {
  bool switchanausVal = digitalRead(OnOffSwitchPin);
  static bool lastswitchanausVal = switchanausVal;
  if (!initComplete)
    return;

  if (lastswitchanausVal == switchanausVal)
    return;
  startRelais(RELAISZUSTAND::AN2000, now);
  lastswitchanausVal = switchanausVal;

  if (debugmode == DEBUGMODE::debug)
    Serial.println("Schalter AN/AUS gedrückt");
}

void readModeSwitch(unsigned long now, bool initComplete) {
  bool switchmodeVal = digitalRead(modeSwitchPin);
  static bool lastswitchmodeVal = switchmodeVal;
  if (!initComplete)
    return;

  if (switchmodeVal == lastswitchmodeVal)
    return;
  startRelais(RELAISZUSTAND::AN500, now);
  lastswitchmodeVal = switchmodeVal;
  if (debugmode == DEBUGMODE::debug)
    Serial.println("Schalter Mode gedrückt");
}

/* Bis jetzt nur einfacher Knopf für Displaymodus / in
   Zukunft vllt gedrückt halten für Laufzeitdaten */
void readDisplayModeSwitch(unsigned long now, bool initComplete) {
  bool current = digitalRead(encoderswPin);
  static bool last = current;
  if (!initComplete)
    return;

  static unsigned long lastdebounce = 0;

  if (current != last) {
    if (current == LOW) { // Taste gedrückt (bei Pullup)
      if (now - lastdebounce > 50) {
        lastdebounce = now;
        displaystatus = (displaystatus == DISPLAYSTATUS::passive)
                            ? DISPLAYSTATUS::standard
                            : DISPLAYSTATUS::passive;
      }
    }
  }
  last = current;
}
void readAndInterpretEncoder() {
  long encoderwert = encoder.read(); // Bibliothek liefert long, inkrementiert
                                     // bei jeder "Zähleinheit" des Encoders
  static long lastencoderwert = 0; // vorheriger Wert, um Differenz zu bestimmen
  static float deltaSumme = 0;
  long diff = encoderwert - lastencoderwert; // Diese Bibliotheksfunktion gibt
                                             // den Wert in 4er Schritten aus!

  if (diff == 0)
    return; // nichts bewegt, also nichts tun

  deltaSumme +=
      diff *
      0.125; // Wird quasi geviertelt um einzelne Schritte Herauszubekommen
             // (Vorher habe ich einfach durch vier geteilt, aber wenn da null
             // raus kommt läuft die ganze Funktion nicht mehr)

  if (Heizungsmode ==
      HEIZUNGSMODE::TEMP) { // Im TEMP Mode (Ich will später vllt noch eine
                            // Ausgabe machen um zu gucken wie viel die Heizung
                            // lief wegen Spritverbrauch etc.)
    while (deltaSumme >=
           0.5) { // while Schleife, damit der code das ganze Delta abarbeitet
                  // bevor er weiter läuft (Vorher habe ich if genutzt aber dann
                  // wurde pro loop nur um 0,5 angepasst, was bei schnellen
                  // Drehungen sehr langsame reaktionen ausgelöst hat.)
      Solltemperatur += 0.5;
      deltaSumme -= 0.5;
    }

    while (deltaSumme <= -0.5) { // Der wert in der Bedingung ist auch eine
                                 // Empfindlichkeitsdarstellung, da sie einen
                                 // minimalen delta Impuls vorraussetzt.
      Solltemperatur -= 0.5;
      deltaSumme += 0.5;
    }
  }

  if (Solltemperatur > 30)
    Solltemperatur = 30;
  else if (Solltemperatur < 5)
    Solltemperatur = 5;

  lastencoderwert = encoderwert;
}

// Wrapperfunktion um alle Schalter auf einmal zu lesen
void readAllSwitches(unsigned long now, bool initComplete) {
  readOnOfSwitch(now, initComplete);
  readModeSwitch(now, initComplete);
  readDisplayModeSwitch(now, initComplete);
  readAndInterpretEncoder(); // braucht keinen Zeitstempel und init weil das
                             // Debouncing und init in der Bibliothek passiert
}

//
// Ende der Schalterfunktionen

void startTemperatureRequest(unsigned long now) { // Beginn der
                                                  // Temperaturmessung
  if (now - lastTempRequest < 1000)
    return;
  sensors.requestTemperatures(); // Startet Messung im Hintergrund
  lastTempRequest = now;
  tempRequestPending = true;
}

void readTemperature(unsigned long now) {
  if (!tempRequestPending)
    return; // Keine Messung angefordert
  if (now - lastTempRequest < tempMeasurementDelay)
    return; // Zeit die Messung braucht noch nicht abgeblaufen

  tempC = sensors.getTempCByIndex(0); // Ergebnis einholen
  tempRequestPending = false;         // Kein Ergebnis mehr ausstehend
}

void temperaturmessung(unsigned long now) {
  if (!tempRequestPending)
    startTemperatureRequest(now); // Wenn keine Messung gestartet, dann starten
  else
    readTemperature(now); // Wenn Messung läuft Ergebnis abholen

  // Raumtemperatur setzen
  if (tempC < -30 || tempC > 80) {
    Raumtemperatur = RAUMTEMPERATUR::Fehler;
    Serial.println("Fehler Temperatursensor");
  } else if (tempC < Solltemperatur - Toleranz_Solltemperatur)
    Raumtemperatur = RAUMTEMPERATUR::kalt;
  else if (tempC > Solltemperatur + Toleranz_Solltemperatur)
    Raumtemperatur = RAUMTEMPERATUR::warm;
  else
    Raumtemperatur = RAUMTEMPERATUR::richtig;
}

void temperaturschaltung(unsigned long now) {
  static RAUMTEMPERATUR lastHandledRaumtemperatur = RAUMTEMPERATUR::richtig;
  if (Raumtemperatur == lastHandledRaumtemperatur)
    return;
  if (Heizungsmode != HEIZUNGSMODE::TEMP)
    return;

  if (Raumtemperatur == RAUMTEMPERATUR::kalt &&
      Heizungszustand == HEIZUNGSZUSTAND::AUS) {
    startRelais(RELAISZUSTAND::AN2000, now);
    // if (debugmode == DEBUGMODE::debug) Serial.println("Relais kalt
    // Schalten");
  } else if (Raumtemperatur == RAUMTEMPERATUR::warm &&
             Heizungszustand == HEIZUNGSZUSTAND::AN) {
    startRelais(RELAISZUSTAND::AN2000, now);
    if (debugmode == DEBUGMODE::debug)
      Serial.println("Relais warm Schalten");
  }
  lastHandledRaumtemperatur = Raumtemperatur;
}

bool initSystems() {
  bool initComplete = false;
  Serial.begin(9600);
  // Initialer Zustand der Schalter einlesen hier
  // millis() weil zeitwertparameter erwartet wird, now
  // ist aber der loop zeitstempel
  readAllSwitches(millis(), initComplete);
  sensors.begin();
  sensors.requestTemperatures();
  tempC = sensors.getTempCByIndex(0);

  if (debugmode == DEBUGMODE::debug) {
    Serial.println("Startup abgeschlossen");
    Serial.print("Temperatur ");
    Serial.println(tempC);
  }

  display_init();

  initComplete = true;
  return initComplete;
}
