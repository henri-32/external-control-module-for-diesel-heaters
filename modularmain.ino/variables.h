// variables.h

#ifndef VARIABLES_H
#define VARIABLES_H

enum class HEIZUNGSZUSTAND : uint8_t { AUS,
                                       AN };


enum class HEIZUNGSMODE : uint8_t { TEMP,
                                    POWER };

enum class RELAISZUSTAND : uint8_t { AUS,
                                     AN500,
                                     AN2000 };

enum class HEIZUNG_STARTBEREIT : uint8_t { nichtbereit,
                                           bereit };

enum class RAUMTEMPERATUR : uint8_t { richtig,
                                      kalt,
                                      warm,
                                      Fehler };

enum class ART_DES_SCHALTENS : uint8_t { nichtgesetzt,
                                         Schalter,
                                         Temperatur };

enum class TEMPERATURSPERRE : uint8_t { nichtaktiv,
                                        aktiv,
                                        abgelaufen };

enum class DEBUGMODE : uint8_t { running,
                                 debug,
                                 oledprint };

extern HEIZUNGSZUSTAND Heizungszustand;
extern HEIZUNGSMODE Heizungsmode;
extern RELAISZUSTAND Relaiszustand;
extern HEIZUNG_STARTBEREIT Heizung_startbereit;
extern RAUMTEMPERATUR Raumtemperatur;
extern ART_DES_SCHALTENS Art_des_Schaltens;
extern TEMPERATURSPERRE Temperatursperre;
extern DEBUGMODE Debugmode;

extern const int switchanausPin;
extern const int switchmodePin;
extern const int ds18b20Pin;
extern const int outputrelaisPin;
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

#endif