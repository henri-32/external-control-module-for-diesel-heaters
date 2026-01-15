// hardwarefunctions.h
#pragma once
#include "variables.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

void relaisstarten(RELAISZUSTAND zielzustand, unsigned long now);
void relaischeck_loesen(unsigned long now);
void anaus_Schalter(unsigned long now);
void modeSwitch(unsigned long now);
void display_switch(unsigned long now);
void startTemperatureRequest(unsigned long now);
void readTemperature(unsigned long now);
