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
public:
  void clear() override { ++clear_calls; };
  void noDisplay() override { ++no_display_calls; };
  void display() override { ++display_calls; };
  void noBacklight() override { ++no_backlight_calls; };
  void backlight() override { ++backlight_calls; };
  void setCursor(uint8_t col, uint8_t row) override {
    cursor_calls.emplace_back(col, row);
  };
  void printstr(const char text[]) override {
    printed_lines.emplace_back(text);
  };

  void init() override;

  bool m_init_called = false;
  int clear_calls = 0;
  int no_display_calls = 0;
  int display_calls = 0;
  int no_backlight_calls = 0;
  int backlight_calls = 0;
  std::vector<std::pair<uint8_t, uint8_t>> cursor_calls;
  std::vector<std::string> printed_lines;
};

class TestEncoder : public IEncoder {
public:
  int read() override;

private:
  int testRead = 0;
};

#pragma GCC diagnostic pop
