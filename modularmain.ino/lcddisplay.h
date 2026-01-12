#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

void display_init();
void display_update_wrapper(unsigned long now);
