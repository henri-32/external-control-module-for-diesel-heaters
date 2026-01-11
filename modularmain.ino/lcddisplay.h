#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

void display_init();
void display_update(const String &line0 = "", const String &line1 = "", const String &line2 = "", const String &line3 = "");
