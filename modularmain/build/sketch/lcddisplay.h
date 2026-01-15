#line 1 "/home/henri-32/Softwareprojekte/Arduinoprojekte/Heizungssteuerung_git/modularmain/lcddisplay.h"
#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

void display_init();
void lcdDisplay(unsigned long now);
enum class DISPLAYSTATUS { standard,
                           passive };
extern DISPLAYSTATUS displaystatus;
