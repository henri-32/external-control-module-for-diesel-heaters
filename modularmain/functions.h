#pragma once
#include "variables.h"
#include <Arduino.h>
#include <DallasTemperature.h>
#include <Encoder.h>
#include <OneWire.h>

void pinSettings();
void initSystems();

// ehemals hardwarefunctions.h
void relaisstarten(RELAISZUSTAND zielzustand, unsigned long now);
void relaischeck_loesen(unsigned long now);
void readOnOfSwitch(unsigned long now);
void readModeSwitch(unsigned long now);
void readDisplayModeSwitch(unsigned long now);
void startTemperatureRequest(unsigned long now);
void readTemperature(unsigned long now);

// ehemals logicfunctions.h
bool requestrelais(unsigned long now, RELAISZUSTAND zielzustand,
                   ART_DES_SCHALTENS quelle);
void readAndInterpretEncoder();
void temperaturmessung(unsigned long now);
void temperaturschaltung(unsigned long now);
void checktemperatursperre(unsigned long now);
void temperatursperre(unsigned long now);
void temperatursperre_reset();