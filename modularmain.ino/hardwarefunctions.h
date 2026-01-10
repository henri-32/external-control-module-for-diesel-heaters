// hardwarefunctions.h
#ifndef HARDWAREFUNCTIONS_H
#define HARDWAREFUNCTIONS_H

void relaisstarten(unsigned long now);
void relaischeck_loesen(unsigned long now);
void anaus_Schalter(unsigned long now);
void mode_schalter(unsigned long now);
void startTemperatureRequest(unsigned long now);
void readTemperature(unsigned long now);

#endif