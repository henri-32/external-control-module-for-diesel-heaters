// modularmain.ino.ino
#pragma once
#include "functions.h"
#include "lcddisplay.h"
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

void setup() {
  pinSettings();
  initSystems();
}

void loop() {
  unsigned long now = millis();
  readOnOfSwitch(now);
  readModeSwitch(now);
  readDisplayModeSwitch(now);
  readAndInterpretEncoder();
  relaischeck_loesen(now);
  checktemperatursperre(now);
  temperaturmessung(now);
  temperaturschaltung(now);
  lcdDisplay(now);
}