#line 1 "/home/henri-32/Softwareprojekte/Arduinoprojekte/Heizungssteuerung_git/modularmain/logicfunctions.h"
//logicfunction.h
#pragma once
#include "variables.h"
#include <Arduino.h>
#include <Encoder.h>

bool requestrelais(unsigned long now, RELAISZUSTAND zielzustand, ART_DES_SCHALTENS quelle = ART_DES_SCHALTENS::Schalter);
void interpretencoder(); 
void debugPrint(unsigned long now);
void temperaturmessung(unsigned long now);
void temperaturschaltung(unsigned long now);
void checktemperatursperre(unsigned long now);
void temperatursperre(unsigned long now);
void temperatursperre_reset();

