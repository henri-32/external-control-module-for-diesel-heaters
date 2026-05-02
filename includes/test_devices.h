#pragma once
#include "interfaces.h"
#include "types.h"
#include <string>
#include <utility>
#include <vector>

class TestInputDevices : public IInputDevices {
public:
  TestInputDevices(ControllerInputData &id);
  void init() override {}
  void updateInputData() override {}
};

class TestOutputDevices : public IOutputDevices {
public:
  TestOutputDevices(ControllerOutputIntent &oi, IDisplay &display);
  void init() override {}
  void update() override {}
  const ControllerOutputIntent &call();

  ControllerOutputIntent &m_outputIntent;
  IDisplay &m_display;
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

class TestDisplay : public IDisplay {
  //{{{
public:
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = 0x00) override {};
  void clear() override { ++clear_calls; };
  void homevirtual() override {};
  void noDisplay() override { ++no_display_calls; };
  void display() override { ++display_calls; };
  void noBlink() override {};
  void blink() override {};
  void noCursor() override {};
  void cursor() override {};
  void scrollDisplayLeft() override {};
  void scrollDisplayRight() override {};
  void leftToRight() override {};
  void rightToLeft() override {};
  void noBacklight() override { ++no_backlight_calls; };
  void backlight() override { ++backlight_calls; };
  void autoscroll() override {};
  void noAutoscroll() override {};
  void createChar(uint8_t, uint8_t[]) override{};
  void createChar(uint8_t location, const char *charmap) override {};
  void setCursor(uint8_t col, uint8_t row) override {
    cursor_calls.emplace_back(col, row);
  };
  void blink_on() override {};   // alias for blink()
  void blink_off() override {};  // alias for noBlink()
  void cursor_on() override {};  // alias for cursor()
  void cursor_off() override {}; // alias for noCursor()
  void setBacklight(uint8_t new_val) override {
  }; // alias for backlight() and nobacklight()
  void load_custom_character(uint8_t char_num, uint8_t *rows) override {
  }; // alias for createChar()
  void printstr(const char text[]) override {
    printed_lines.emplace_back(text);
  };

  void init() override;
  void update() override {};

  bool m_init_called = false;
  int clear_calls = 0;
  int no_display_calls = 0;
  int display_calls = 0;
  int no_backlight_calls = 0;
  int backlight_calls = 0;
  std::vector<std::pair<uint8_t, uint8_t>> cursor_calls;
  std::vector<std::string> printed_lines;
};
//}}}
#pragma GCC diagnostic pop
