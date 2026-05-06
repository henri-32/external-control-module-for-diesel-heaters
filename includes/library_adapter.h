// Die in dieser Datei deklarierten Adapter geben lediglich Aufrufe an Libraries
// weiter. Sie dienen dem Zweck Sensoren virtualisieren zu können.
#pragma once
#include "interfaces.h"
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class LCDAdapter : public IDisplay {
public:
  LCDAdapter(uint8_t lcdAddr, uint8_t lcd_cos, uint8_t lcd_rows)
      : m_lcdAddr(lcdAddr), m_lcd_cos(lcd_cos), m_lcd_rows(lcd_rows){};

  void clear() override;
  void noDisplay() override;
  void display() override;
  void noBacklight() override;
  void backlight() override;
  void setCursor(uint8_t, uint8_t) override;
  void printstr(const char[]) override;

  void init() override;

private:
  uint8_t m_lcdAddr;
  uint8_t m_lcd_cos;
  uint8_t m_lcd_rows;
  LiquidCrystal_I2C liquidCristal_I2C{m_lcdAddr, m_lcd_cos, m_lcd_rows};
};
//}}}

class EncoderAdapter : public IEncoder {
public:
  ~EncoderAdapter() = default;
  EncoderAdapter(int pin1, int pin2) : libraryObject(pin1, pin2) {}
  int read() override;

private:
  Encoder libraryObject; // Encoder library genutzt: Dort findet Pin Zuordnung statt
};

class TempSensorAdapter : public ITempSensor {
public: 
  TempSensorAdapter(OneWire& ow) : libraryObject{&ow} {};
  
  void begin() override; 
  void requestTemperatures() override; 
  float getTempCByIndex(uint8_t index) override; 

private: 
  DallasTemperature libraryObject;
};
