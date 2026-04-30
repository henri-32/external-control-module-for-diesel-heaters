#include "interfaces.h"
#include "types.h"

class TestInputDevices : public IInputDevices {
public:
  TestInputDevices(ControllerInputData &id);
  void init() override {}
  void updateInputData() override {}
};

class TestOutputDevices : public IOutputDevices {
public:
  TestOutputDevices(ControllerOutputIntent &oi);
  void init() override {}
  void update() override {}
};

class TestDisplay : public IDisplay {
  //{{{
public:
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = 0x00) override;
  void clear() override;
  void noDisplay() override;
  void display() override;
  void noBlink() override;
  void blink() override;
  void noCursor() override;
  void cursor() override;
  void scrollDisplayLeft() override;
  void scrollDisplayRight() override;
  void printLeft() override;
  void printRight() override;
  void leftToRight() override;
  void rightToLeft() override;
  void shiftIncrement() override;
  void shiftDecrement() override;
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
};
//}}}
