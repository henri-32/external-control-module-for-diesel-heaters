#pragma once
#include "interfaces.h"
#include "types.h"
#include <string>
#include <utility>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

class TestInputDevices : public IInputDevices {
  //{{{
  void init() override;
  void update() override;
};
//}}}

class TestOutputDevices : public IOutputDevices {
  //{{{
  void init() override;
  void update() override;
};
//}}}

class TestDisplay : public IDisplay {
  //{{{
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
//}}}

class TestRelais : public IRelais {
  //{{{
public:
  void init() override{};
  void update(OutputDevicesIntent::RelaisCommand intent) override{};
};
//}}}

class TestEncoderHardware : public IEncoderHardware {
  //{{{
public:
  int read() override;

  int position = 0;
};
//}}}

class TestToggleSwitch : public IToggleSwitch {
  //{{{
public:
  TestToggleSwitch() = default;
  void init() override{};
  bool changed() override { return false; };
};
//}}}

class TestPushButton : public IPushButton {
  //{{{
public:
  TestPushButton() = default;
  void init() override{};
  bool isDown() const override { return false; };
  bool released() override { return false; };
};
//}}}

class TestTemperatureSensorHardware : public ITempSensorHardware {
  //{{{
public:
  TestTemperatureSensorHardware() = default;
  void begin() override{};
  void requestTemperatures() override{};
  float getTempCByIndex(uint8_t index) override;
  void setTempReturn(float temp);

private:
  float m_tempReturn = 0;
};
//}}}

#pragma GCC diagnostic pop
