//logicfunction.h
#include "hardwarefunctions.h"
#include "variables.h"
#include <Arduino.h>
#include <Encoder.h>


#ifndef LOGICFUNCTIONS_H
#define LOGICFUNCTIONS_H

bool requestrelais(unsigned long now, RELAISZUSTAND zielzustand, ART_DES_SCHALTENS quelle = ART_DES_SCHALTENS::Schalter);
void interpretencoder(); 
void debugprint(unsigned long now);
void temperaturmessung(unsigned long now);
void temperaturschaltung(unsigned long now);
void checktemperatursperre(unsigned long now);
void temperatursperre(unsigned long now);
void temperatursperre_reset();




#endif