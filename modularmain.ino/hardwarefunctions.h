// hardwarefunctions.h
#pragma once
#include "variables.h"
#include "logicfunctions.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

void relaisstarten(RELAISZUSTAND zielzustand, unsigned long now);
void relaischeck_loesen(unsigned long now);
void anaus_Schalter(unsigned long now);
void mode_Schalter(unsigned long now);
void startTemperatureRequest(unsigned long now);
void readTemperature(unsigned long now);
