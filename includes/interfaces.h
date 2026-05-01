#pragma once
#include "stdint.h"
#include "types.h"

class IInputDevices {
public:
  virtual void init() = 0;
  virtual void updateInputData() = 0;
};

class IOutputDevices {
public:
  virtual void init() = 0;
  virtual void update() = 0;
};

class IDisplay {
  //{{{
public:
  virtual void begin(uint8_t cols, uint8_t rows, uint8_t charsize = 0x00) = 0;
  virtual void clear() = 0;
  virtual void homevirtual() = 0;
  virtual void noDisplay() = 0;
  virtual void display() = 0;
  virtual void noBlink() = 0;
  virtual void blink() = 0;
  virtual void noCursor() = 0;
  virtual void cursor() = 0;
  virtual void scrollDisplayLeft() = 0;
  virtual void scrollDisplayRight() = 0;
  virtual void leftToRight() = 0;
  virtual void rightToLeft() = 0;
  virtual void noBacklight() = 0;
  virtual void backlight() = 0;
  virtual void autoscroll() = 0;
  virtual void noAutoscroll() = 0;
  virtual void createChar(uint8_t, uint8_t[]) = 0;
  virtual void createChar(uint8_t location, const char *charmap) = 0;
  virtual void setCursor(uint8_t, uint8_t) = 0;
  virtual void blink_on() = 0;   // alias for blink()
  virtual void blink_off() = 0;  // alias for noBlink()
  virtual void cursor_on() = 0;  // alias for cursor()
  virtual void cursor_off() = 0; // alias for noCursor()
  virtual void
  setBacklight(uint8_t new_val) = 0; // alias for backlight() and nobacklight()
  virtual void
  load_custom_character(uint8_t char_num,
                        uint8_t *rows) = 0; // alias for createChar()
  virtual void printstr(const char[]) = 0;

  virtual void init() = 0;
  virtual void update() = 0;
};
//}}}
//}}}
