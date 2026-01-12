//modularmain.ino.ino
#pragma once
#include "variables.h"
#include "hardwarefunctions.h"
#include "logicfunctions.h"
#include "lcddisplay.h"

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//Bibliotheken verknüpfen


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
  display_update_wrapper(now)