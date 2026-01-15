//logicfunctions.cpp
#include "logicfunctions.h"
#include "hardwarefunctions.h"

// ---------------- KOMMANDOZENTRALE ----------------
bool requestrelais(unsigned long now, RELAISZUSTAND zielzustand, ART_DES_SCHALTENS quelle = ART_DES_SCHALTENS::Schalter) {
  uint8_t keysteuerung = (static_cast<uint8_t>(zielzustand) << 2) | static_cast<uint8_t>(quelle);
  switch (keysteuerung) {
    case 0b1010:  // AN2000 + Temperatur
      if (Heizung_startbereit == HEIZUNG_STARTBEREIT::nichtbereit) return false;
      if (Relaiszustand != RELAISZUSTAND::AUS) return false;
      if (Temperatursperre == TEMPERATURSPERRE::aktiv) return false;
      relaisstarten(RELAISZUSTAND::AN2000, now);
      return true;
    case 0b1001:  // AN2000 + Schalter
      if (Heizung_startbereit == HEIZUNG_STARTBEREIT::nichtbereit) return false;
      if (Relaiszustand != RELAISZUSTAND::AUS) return false;
      relaisstarten(RELAISZUSTAND::AN2000, now);
      return true;
    case 0b0101:  // AN500 + Schalter
      if (Heizung_startbereit == HEIZUNG_STARTBEREIT::nichtbereit) return false;
      if (now - lastDebounceTimemode < debounceDelay) return false;
      relaisstarten(RELAISZUSTAND::AN500, now);
      return true;
  }
  return false;
}

void interpretencoder() {
  long encoderwert = encoder.read();  // Bibliothek liefert long, inkrementiert bei jeder "Zähleinheit" des Encoders
  static long lastencoderwert = 0;    // vorheriger Wert, um Differenz zu bestimmen
  static float deltaSumme = 0;
  long diff = encoderwert - lastencoderwert;  // Diese Bibliotheksfunktion gibt den Wert in 4er Schritten aus!

  if (diff == 0) return;  // nichts bewegt, also nichts tun

  deltaSumme += diff * 0.125;  // Wird quasi geviertelt um einzelne Schritte Herauszubekommen (Vorher habe ich einfach durch vier geteilt, aber wenn da null raus kommt läuft die ganze Funktion nicht mehr)

  if (Heizungsmode == HEIZUNGSMODE::TEMP) {  // Im TEMP Mode
    while (deltaSumme >= 0.5) {              // while Schleife, damit der code das ganze Delta abarbeitet bevor er weiter läuft (Vorher habe ich if genutzt aber dann wurde pro loop nur um 0,5 angepasst, was bei schnellen Drehungen sehr langsame reaktionen ausgelöst hat.)
      Solltemperatur += 0.5;
      deltaSumme -= 0.5;
    }

    while (deltaSumme <= -0.5) {            // Der wert in der Bedingung ist auch eine Empfindlichkeitsdarstellung, da sie einen minimalen delta Impuls vorraussetzt. 
      Solltemperatur -= 0.5;
      deltaSumme += 0.5;
    }
  }

  if (Solltemperatur > 30) Solltemperatur = 30;
  else if (Solltemperatur < 5) Solltemperatur = 5;

  lastencoderwert = encoderwert;
}


void debugprint(unsigned long now) {
  if (debugmode != DEBUGMODE::debug) return;
  if (now - lastdebugprint < debugprintintervall) return;

  lastdebugprint = now;

  Serial.println("-----STATUS-----");
  Serial.print("Temp: ");
  Serial.print(tempC);
  Serial.println(" C");
  Serial.print("Soll: ");
  Serial.println(Solltemperatur);
  Serial.print("Heizung: ");
  switch (Heizungszustand) {
    case HEIZUNGSZUSTAND::AUS: Serial.println("Heizungszustand AUS"); break;
    case HEIZUNGSZUSTAND::AN: Serial.println("Heizungszustand AN"); break;
  }
  Serial.print("Modus: ");
  Serial.println(Heizungsmode == HEIZUNGSMODE::TEMP ? "TEMP" : "POWER");
  Serial.print("Raumstatus: ");
  switch (Raumtemperatur) {
    case RAUMTEMPERATUR::richtig: Serial.println("Raumtemperatur richtig"); break;
    case RAUMTEMPERATUR::kalt: Serial.println("Raumtemperatur kalt"); break;
    case RAUMTEMPERATUR::warm: Serial.println("Raumtemperatur warm"); break;
    case RAUMTEMPERATUR::Fehler: Serial.println("Raumtemperatur Fehler"); break;
  }
  if (Temperatursperre == TEMPERATURSPERRE::aktiv) Serial.println("Temperatursperre aktiv");
  Serial.println("------------------");
}


void temperaturmessung(unsigned long now) {
  if (!tempRequestPending) startTemperatureRequest(now);  // Wenn keine Messung gestartet, dann starten
  else readTemperature(now);                              // Wenn Messung läuft Ergebnis abholen

  //Raumtemperatur setzen
  if (tempC < -30 || tempC > 80) {
    Raumtemperatur = RAUMTEMPERATUR::Fehler;
    Serial.println("Fehler Temperatursensor");
  } else if (tempC < Solltemperatur - Toleranz_Solltemperatur) Raumtemperatur = RAUMTEMPERATUR::kalt;
  else if (tempC > Solltemperatur + Toleranz_Solltemperatur) Raumtemperatur = RAUMTEMPERATUR::warm;
  else Raumtemperatur = RAUMTEMPERATUR::richtig;
}

void temperaturschaltung(unsigned long now) {
  static RAUMTEMPERATUR lastHandledRaumtemperatur = RAUMTEMPERATUR::richtig;
  if (Raumtemperatur == lastHandledRaumtemperatur) return;
  if (Heizung_startbereit == HEIZUNG_STARTBEREIT::nichtbereit) return;
  if (Temperatursperre == TEMPERATURSPERRE::aktiv) return;
  if (Heizungsmode != HEIZUNGSMODE::TEMP) return;

  if (Raumtemperatur == RAUMTEMPERATUR::kalt && Heizungszustand == HEIZUNGSZUSTAND::AUS) {
    requestrelais(now, RELAISZUSTAND::AN2000, ART_DES_SCHALTENS::Temperatur);
    //if (debugmode == DEBUGMODE::debug) Serial.println("Relais kalt Schalten");
  } else if (Raumtemperatur == RAUMTEMPERATUR::warm && Heizungszustand == HEIZUNGSZUSTAND::AN) {
    requestrelais(now, RELAISZUSTAND::AN2000, ART_DES_SCHALTENS::Temperatur);
    if (debugmode == DEBUGMODE::debug) Serial.println("Relais warm Schalten");
  }
  lastHandledRaumtemperatur = Raumtemperatur;
}

void checktemperatursperre(unsigned long now) {
  if (Temperatursperre == TEMPERATURSPERRE::aktiv && now - beginnsperretimer > dauersperre) {
    temperatursperre_reset();
  }
}
void temperatursperre(unsigned long now) {
  Temperatursperre = TEMPERATURSPERRE::aktiv;
  beginnsperretimer = now;
}

void temperatursperre_reset() {
  Temperatursperre = TEMPERATURSPERRE::nichtaktiv;
  beginnsperretimer = 0;
}
