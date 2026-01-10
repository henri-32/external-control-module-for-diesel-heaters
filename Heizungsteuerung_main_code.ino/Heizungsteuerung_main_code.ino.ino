// // ------------Bibliotheken------------ // Hinzufügen des LCDs
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Encoder.h>

// // ------------Ein- und Ausgänge-------------
const int switchanausPin = 2;
const int switchmodePin = 3;
const int ds18b20Pin = 5;
const int outputrelaisPin = 4;
const int TRAPin = 6;
const int TRBPin = 7;
const int encoderswPin =8;

// //-------------Bibliotheken verknüpfen--------
OneWire oneWire(ds18b20Pin);
DallasTemperature sensors(&oneWire);

// ------------Bools-----------------
bool tempRequestPending = false;  //Temperaturmessung angefordert Ergebnis ausstehend

// ------------Variablen---------------
float tempC;
float Solltemperatur = 20.0;
const float Toleranz_Solltemperatur = 1.5;
Encoder encoder(TRAPin, TRBPin);

//-------------Zeitvariablen ------------
unsigned long lastDebounceTimeanaus = 0;
unsigned long lastDebounceTimemode = 0;
const unsigned long debounceDelay = 50;

unsigned long lastdebugprint = 0;
const unsigned long debugprintintervall = 3000;

unsigned long lastrelais2000time = 0;
unsigned long lastrelais500time = 0;
const unsigned long relais2000delay = 2000;
const unsigned long relais500delay = 500;

unsigned long beginnsperretimer = 0;
const unsigned long dauersperre = 360000;

unsigned long lastTempRequest = 0;
const unsigned long tempMeasurementDelay = 750;  // für 12-Bit DS18B20


// ------------Zustände-------------
enum class HEIZUNGSZUSTAND : uint8_t { AUS,
                                       AN };
HEIZUNGSZUSTAND Heizungszustand = HEIZUNGSZUSTAND::AUS;

enum class HEIZUNGSMODE : uint8_t { TEMP,
                                    POWER };
HEIZUNGSMODE Heizungsmode = HEIZUNGSMODE::TEMP;

enum class RELAISZUSTAND : uint8_t { AUS,
                                     AN500,
                                     AN2000 };
RELAISZUSTAND Relaiszustand = RELAISZUSTAND::AUS;

enum class HEIZUNG_STARTBEREIT : uint8_t { nichtbereit,
                                           bereit };
HEIZUNG_STARTBEREIT Heizung_startbereit = HEIZUNG_STARTBEREIT::nichtbereit;

enum class RAUMTEMPERATUR : uint8_t { richtig,
                                      kalt,
                                      warm,
                                      Fehler };
RAUMTEMPERATUR Raumtemperatur = RAUMTEMPERATUR::richtig;

enum class ART_DES_SCHALTENS : uint8_t { nichtgesetzt,
                                         Schalter,
                                         Temperatur };
ART_DES_SCHALTENS Art_des_Schaltens = ART_DES_SCHALTENS::nichtgesetzt;

enum class TEMPERATURSPERRE : uint8_t { nichtaktiv,
                                        aktiv };
TEMPERATURSPERRE Temperatursperre = TEMPERATURSPERRE::nichtaktiv;

enum class DEBUGMODE : uint8_t { running,
                                 debug,
                                 oledprint };
DEBUGMODE debugmode = DEBUGMODE::debug;

// ------------ Allgemeine Funktionen------------

void setup() {
  pinMode(switchanausPin, INPUT_PULLUP);
  pinMode(switchmodePin, INPUT_PULLUP);
  pinMode(ds18b20Pin, INPUT);  // externer Pullup
  pinMode(outputrelaisPin, OUTPUT);
  pinMode(TRAPin, INPUT);
  pinMode(TRBPin, INPUT);
  pinMode (encoderswPin, INPUT_PULLUP);
  digitalWrite(outputrelaisPin, LOW);

  Serial.begin(9600);
  sensors.begin();
  sensors.requestTemperatures();
  tempC = sensors.getTempCByIndex(0);

  Heizung_startbereit = HEIZUNG_STARTBEREIT::bereit;

  if (debugmode == DEBUGMODE::debug) {
    Serial.println("Startup abgeschlossen");
    Serial.print("Temperatur ");
    Serial.println(tempC);
  }
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

void temperatursperre(unsigned long now) {
  Temperatursperre = TEMPERATURSPERRE::aktiv;
  beginnsperretimer = now;
}

void temperatursperre_reset() {
  Temperatursperre = TEMPERATURSPERRE::nichtaktiv;
  beginnsperretimer = 0;
}

void checktemperatursperre(unsigned long now) {
  if (Temperatursperre == TEMPERATURSPERRE::aktiv && now - beginnsperretimer > dauersperre) {
    temperatursperre_reset();
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

void interpretencoder() {             // Drehaktionen lösen je nach Modus andere Aktionen aus, müssen daher in Kontext gesetzt werden
  long encoderwert = encoder.read();  // Die Funktion gibt einen Wert zurück, der mit jeder Drehung aktualisiert wird. Aufgrund des physischen encoders sind es vier Schritte für einen Klick
  static long lastencoderwert = 0;    // Es muss beides long sein, weil die Bibliothek, long zurückgeben kann
  if (encoderwert == lastencoderwert) return;

  int8_t encoderaction = (encoderwert - lastencoderwert) / 4;  // Die vier Schritte eines Klicks werden auf eine action heruntergebrochen.


  if (Heizungsmode == HEIZUNGSMODE::TEMP) {                 // Im TEMP Mode wird die Solltemperatur eingestellt noch keine Weiteren Funktionen da kein LCD Display
    Solltemperatur = Solltemperatur + encoderaction * 0.5;  // Die Solltemperatur wird um die Anzahl der Klicks auf dem Encoder in 0,5 grad Schritten eingestellt. Es ist keine Unterscheidungen mehr nötig, da encoderaction bereits signed ist
    if(Solltemperatur > 30) Solltemperatur =30; // Begrenzung der maximalen Werte der Solltemperatur
    else if (Solltemperatur <5) Solltemperatur =5;
  }
  lastencoderwert = encoderwert;
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

void tratest(){
  if (digitalRead(TRAPin))Serial.println("TRA");
}
void loop() {
  unsigned long now = millis();
  anaus_Schalter(now);
  mode_Schalter(now);
  interpretencoder();
  relaischeck_loesen(now);
  checktemperatursperre(now);
  temperaturmessung(now);
  temperaturschaltung(now);
  debugprint(now);
}