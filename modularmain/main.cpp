// modularmain.ino.ino
#pragma once
#include "functions.h"
#include "lcddisplay.h"
#include "classes.h"
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
 
bool initComplete;

void setup() {
  pinSettings();
  initComplete = initSystems();
}

void loop() {
  unsigned long now = millis();
  readAllSwitches(now, initComplete);
  relaischeck_loesen(now);
  temperaturmessung(now);
  temperaturschaltung(now);
  lcdDisplay(now);
}