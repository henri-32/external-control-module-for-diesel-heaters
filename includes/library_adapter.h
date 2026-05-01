//Die in dieser Datei deklarierten Adapter geben lediglich Aufrufe an Libraries weiter. 
//Sie dienen dem Zweck Sensoren virtualisieren zu können. 
#pragma once
#include "interfaces.h"
#include <LiquidCrystal_I2C.h>

class LCDAdapter : public IDisplay {
//{{{
public:
  LCDAdapter(uint8_t lcdAddr, uint8_t lcd_cos, uint8_t lcd_rows)
      : m_lcdAddr(lcdAddr), m_lcd_cos(lcd_cos), m_lcd_rows(lcd_rows){};

  void begin(uint8_t cols, uint8_t rows,
                     uint8_t charsize = 0x00) override;
  void clear() override;
  void homevirtual() override;
  void noDisplay() override;
  void display() override;
  void noBlink() override;
  void blink() override;
  void noCursor() override;
  void cursor() override;
  void scrollDisplayLeft() override;
  void scrollDisplayRight() override;
  void leftToRight() override;
  void rightToLeft() override;
  void noBacklight() override;
  void backlight() override;
  void autoscroll() override;
  void noAutoscroll() override;
  void createChar(uint8_t, uint8_t[]) override;
  void createChar(uint8_t location, const char *charmap) override;
  void setCursor(uint8_t, uint8_t) override;
  void blink_on() override;   // alias for blink()
  void blink_off() override;  // alias for noBlink()
  void cursor_on() override;  // alias for cursor()
  void cursor_off() override; // alias for noCursor()
  void setBacklight(
      uint8_t new_val) override; // alias for backlight() and nobacklight()
  void load_custom_character(uint8_t char_num,
                             uint8_t *rows) override; // alias for createChar()
  void printstr(const char[]) override;

  void init() override ;
  void update() override ;

private:
  uint8_t m_lcdAddr;
  uint8_t m_lcd_cos;
  uint8_t m_lcd_rows;
  LiquidCrystal_I2C liquidCristal_I2C{m_lcdAddr, m_lcd_cos, m_lcd_rows};
};
//}}}

