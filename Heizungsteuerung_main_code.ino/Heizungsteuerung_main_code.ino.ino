// // ------------Bibliotheken------------ // GIT Test Experimente
#include <OneWire.h>
#include <DallasTemperature.h>

// // ------------Ein- und Ausgänge-------------
const int switchanausPin = 2;
const int switchmodePin = 3;
const int ds18b20Pin = 5;
const int outputrelaisPin = 4;
const int TRAPin = 6;
const int TRBPin = 7;

// //-------------Bibliotheken verknüpfen--------
OneWire oneWire(ds18b20Pin);
DallasTemperature sensors(&oneWire);

// ------------Bools-----------------
bool tempRequestPending = false;  //Temperaturmessung angefordert Ergebnis ausstehend
// ------------Variablen---------------
float tempC;
float Solltemperatur = 20.0;
const float Toleranz_Solltemperatur = 1.5;

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
                                        aktiv,
                                        abgelaufen };
TEMPERATURSPERRE Temperatursperre = TEMPERATURSPERRE::nichtaktiv;

enum class DEBUGMODE : uint8_t { running,
                                 debug,
                                 oledprint };
DEBUGMODE debugmode = DEBUGMODE::debug;

// ------------ Allgemeine Funktionen------------
constexpr uint16_t key_temp(HEIZUNGSZUSTAND h, RAUMTEMPERATUR r);  // optional löschen, wenn nicht verwendet

void setup() {
  pinMode(switchanausPin, INPUT_PULLUP);
  pinMode(switchmodePin, INPUT_PULLUP);
  pinMode(ds18b20Pin, INPUT);  // externer Pullup
  pinMode(outputrelaisPin, OUTPUT);
  pinMode(TRAPin, INPUT_PULLUP);
  pinMode(TRBPin, INPUT_PULLUP);
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

void debug(unsigned long now) {
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

uint8_t encoderauslesen() {
  static bool initialized = false;
  uint8_t encoderposition = 0;
  static uint8_t lastencoderposition = 0;
  uint8_t encodertrigger = 0;  // Die Rückgabe der Funktion ist entweder triggerup oder fiktiv trigger down. Es gibt keine anderen Aktionen. Deswegen als bool
  static bool lastTRAVal;
  bool TRAVal = digitalRead(TRAPin);
  bool TRBVal = digitalRead(TRBPin);

  if (!initialized) {
    lastTRAVal = TRAVal;
    initialized = true;
    return;
  }

  else {
    if (TRAVal != lastTRAVal) {
      if (TRAVal == TRBVal) encoderposition--;
      else encoderposition++;
    }
    lastTRAVal = TRAVal;

    if (encoderposition - lastencoderposition == 2 || encoderposition - lastencoderposition == -2) {  //Wenn sich die Encoderposition um  2 verändert (Hardwarebedingt ein klick zwei Werte)
      if (encoderposition < lastencoderposition) {
        lastencoderposition = encoderposition;
        return encodertrigger = -1;  // -1 ist verringern 1 ist vergrößern und 0 ist keine änderung (nur zur Sicherheit eigentlich sollte bei keiner änderung vorher die funktion returned werden)
      } else {
        lastencoderposition = encoderposition;
        return encodertrigger = 1;
      }
    } else {
      lastencoderposition = encoderposition;
      return 0;
    }
  }
}

void interpretencoder() {  // Drehaktionen lösen je nach Modus andere Aktionen aus, müssen daher in Kontext gesetzt werden
  uint8_t encodertrigger = encoderauslesen();
  if (encodertrigger == 0) return;  // Nur wenn es eine Encoderaktion gibt muss diese interpretiert werden.

  if (Heizungsmode == HEIZUNGSMODE::TEMP) {  // Im TEMP Mode wird die Solltemperatur eingestellt
    if (encodertrigger == -1) Solltemperatur = Solltemperatur - 0, 5;
    else Solltemperatur = Solltemperatur + 0, 5;
  }
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

void loop() {
  unsigned long now = millis();
  anaus_Schalter(now);
  mode_Schalter(now);
  interpretencoder();
  relaischeck_loesen(now);
  checktemperatursperre(now);
  temperaturmessung(now);
  temperaturschaltung(now);
  debug(now);
}