#pragma once
#include "variables.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Encoder.h>

//ehemals hardwarefunctions.h
void relaisstarten(RELAISZUSTAND zielzustand, unsigned long now);
void relaischeck_loesen(unsigned long now);
void anaus_Schalter(unsigned long now);
void modeSwitch(unsigned long now);
void displayModeSwitch(unsigned long now);
void startTemperatureRequest(unsigned long now);
void readTemperature(unsigned long now);

//ehemals logicfunctions.h
bool requestrelais(unsigned long now, RELAISZUSTAND zielzustand, ART_DES_SCHALTENS quelle);
void interpretencoder(); 
void debugPrint(unsigned long now);
void temperaturmessung(unsigned long now);
void temperaturschaltung(unsigned long now);
void checktemperatursperre(unsigned long now);
void temperatursperre(unsigned long now);
void temperatursperre_reset();