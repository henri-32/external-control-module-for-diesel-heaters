//hardwarefunctions.cpp
#include "hardwarefunctions.h"
#include "lcddisplay.h"

//Bibliotheken verknüpfen
OneWire oneWire(ds18b20Pin);
DallasTemperature sensors(&oneWire);


void relaisstarten(RELAISZUSTAND zielzustand, unsigned long now) {
  if (zielzustand == RELAISZUSTAND::AN2000) {
    if (now - lastDebounceTimeanaus < debounceDelay) return;
    digitalWrite(outputrelaisPin, HIGH);
    lastDebounceTimeanaus = now;
    lastrelais2000time = now;
    Relaiszustand = zielzustand;
    Heizungszustand == HEIZUNGSZUSTAND::AUS ? Heizungszustand = HEIZUNGSZUSTAND::AN : Heizungszustand = HEIZUNGSZUSTAND::AUS;
    if (debugmode == DEBUGMODE::debug) Serial.println("Relaistart Zielzustand AN2000");
  } else if (zielzustand == RELAISZUSTAND::AN500) {
    if (now - lastDebounceTimemode < debounceDelay) return;
    digitalWrite(outputrelaisPin, HIGH);
    lastDebounceTimemode = now;
    lastrelais500time = now;
    Relaiszustand = zielzustand;
    Heizungsmode = (Heizungsmode == HEIZUNGSMODE::TEMP) ? HEIZUNGSMODE::POWER : HEIZUNGSMODE::TEMP;
    if (debugmode == DEBUGMODE::debug) Serial.println("Relaistart Zielzustand AN500");
  }
}

void relaischeck_loesen(unsigned long now) {
  if (Relaiszustand == RELAISZUSTAND::AUS) return;

  if (Relaiszustand == RELAISZUSTAND::AN500 && now - lastrelais500time > relais500delay) {
    digitalWrite(outputrelaisPin, LOW);
    Relaiszustand = RELAISZUSTAND::AUS;
  }

  if (Relaiszustand == RELAISZUSTAND::AN2000 && now - lastrelais2000time > relais2000delay) {
    digitalWrite(outputrelaisPin, LOW);
    Relaiszustand = RELAISZUSTAND::AUS;
    if (debugmode == DEBUGMODE::debug) Serial.println("Relais gelöst");
  }
}

void anaus_Schalter(unsigned long now) {
  static bool initialized = false;
  bool switchanausVal = digitalRead(switchanausPin);
  static bool lastswitchanausVal;

  if (!initialized) {
    lastswitchanausVal = switchanausVal;
    initialized = true;
    return;
  } else {
    if (lastswitchanausVal == switchanausVal) return;
    requestrelais(now, RELAISZUSTAND::AN2000, ART_DES_SCHALTENS::Schalter);
    lastswitchanausVal = switchanausVal;

    if (Heizungszustand == HEIZUNGSZUSTAND::AN) temperatursperre(now);
    else if (Heizungszustand == HEIZUNGSZUSTAND::AUS) temperatursperre_reset();
    if (debugmode == DEBUGMODE::debug) Serial.println("Schalter AN/AUS gedrückt");
  }
}

void mode_Schalter(unsigned long now) {
  static bool initialized = false;
  bool switchmodeVal = digitalRead(switchmodePin);
  static bool lastswitchmodeVal;

  if (!initialized) {
    lastswitchmodeVal = switchmodeVal;
    initialized = true;
    return;
  }

  else {
    if (switchmodeVal == lastswitchmodeVal) return;
    requestrelais(now, RELAISZUSTAND::AN500);
    lastswitchmodeVal = switchmodeVal;
    if (debugmode == DEBUGMODE::debug) Serial.println("Schalter Mode gedrückt");
  }
}
void display_switch(unsigned long now) {
  bool current = digitalRead(encoderswPin);
  static bool last = HIGH;
  static unsigned long lastdebounce = 0;

  if (current != last) {
    if (current == LOW) {  // Taste gedrückt (bei Pullup)
      if (now - lastdebounce > 50) {
        lastdebounce = now;
        displaystatus =
          (displaystatus == DISPLAYSTATUS::passive)
            ? DISPLAYSTATUS::standard
            : DISPLAYSTATUS::passive;
      }
    }
  }
  last = current;
}


void startTemperatureRequest(unsigned long now) {  //Beginn der Temperaturmessung
  if (now - lastTempRequest < 1000) return;
  sensors.requestTemperatures();  // Startet Messung im Hintergrund
  lastTempRequest = now;
  tempRequestPending = true;
}

void readTemperature(unsigned long now) {
  if (!tempRequestPending) return;                           // Keine Messung angefordert
  if (now - lastTempRequest < tempMeasurementDelay) return;  // Zeit die Messung braucht noch nicht abgeblaufen
  if (Heizung_startbereit == HEIZUNG_STARTBEREIT::nichtbereit) return;

  tempC = sensors.getTempCByIndex(0);  //Ergebnis einholen
  tempRequestPending = false;          // Kein Ergebnis mehr ausstehend
}
