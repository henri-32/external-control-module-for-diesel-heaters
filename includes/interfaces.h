#pragma once
#include "stdint.h"
#include "types.h"
class IInputDevices {
public:
  virtual void init() = 0;
  virtual void updateInputData() = 0;
};


class IDisplay {
  //{{{
public:
  virtual void begin(uint8_t cols, uint8_t rows, uint8_t charsize = 0x00);
  virtual void clear();
  virtual void homevirtual();
  virtual void noDisplay();
  virtual void display();
  virtual void noBlink();
  virtual void blink();
  virtual void noCursor();
  virtual void cursor();
  virtual void scrollDisplayLeft();
  virtual void scrollDisplayRight();
  virtual void printLeft();
  virtual void printRight();
  virtual void leftToRight();
  virtual void rightToLeft();
  virtual void shiftIncrement();
  virtual void shiftDecrement();
  virtual void noBacklight();
  virtual void backlight();
  virtual void autoscroll();
  virtual void noAutoscroll();
  virtual void createChar(uint8_t, uint8_t[]);
  virtual void createChar(uint8_t location, const char *charmap);
  virtual void setCursor(uint8_t, uint8_t);
  virtual void blink_on();   // alias for blink()
  virtual void blink_off();  // alias for noBlink()
  virtual void cursor_on();  // alias for cursor()
  virtual void cursor_off(); // alias for noCursor()
  virtual void
  setBacklight(uint8_t new_val); // alias for backlight() and nobacklight()
  virtual void load_custom_character(uint8_t char_num,
                                     uint8_t *rows); // alias for createChar()
  virtual void printstr(const char[]);

  virtual void init() = 0;
  virtual void update() = 0;
};
//}}}

class IOutputDevices {
public:
  IOutputDevices(ControllerOutputIntent& oi, IDisplay& display);
  virtual void init() = 0;
  virtual void update() = 0;
};
