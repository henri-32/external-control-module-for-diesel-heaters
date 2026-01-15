#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
void display_init();
void lcdDisplay(unsigned long now);

enum class DISPLAYSTATUS { standard,
                           passive };
extern DISPLAYSTATUS displaystatus;
