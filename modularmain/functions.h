#pragma once
#include "variables.h"
#include <Arduino.h>
#include <DallasTemperature.h>
#include <Encoder.h>
#include <OneWire.h>

void pinSettings();
bool initSystems();

// ehemals hardwarefunctions.h
void startRelais(RELAISZUSTAND zielzustand, unsigned long now);
void readAllSwitches(unsigned long now, bool initComplete);
void relaischeck_loesen(unsigned long now);
void startTemperatureRequest(unsigned long now);
void readTemperature(unsigned long now);

// ehemals logicfunctions.h);
void temperaturmessung(unsigned long now);
void temperaturschaltung(unsigned long now);