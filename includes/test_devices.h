#pragma once
#include "interfaces.h"
#include "pch_test.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

class TestInputDevices : public IInputDevices {
public:
  TestInputDevices(InputDevicesDataSet &inputData) : IInputDevices(inputData) {}
  //{{{
  // This class is intended to isolate the testing of the controller
  // by giving direct access to the inputData and outputIntent for testing
  // the real class InputDevices works with interfaces so it's possible
  ////to use the real class with HardwareStubs. That is done at the
  // Integration Test of SystemController but requires all HardwareStubs
  // to be initialised and denies direct access to the I/O structs

  void init() override{};
  void update() override{};
};
//}}}

class TestOutputDevices : public IOutputDevices {
  //{{{
  // See Comment in TestInputDevices
public:
  TestOutputDevices(OutputDevicesIntent &outputIntent, IRelais &relais)
      : IOutputDevices(outputIntent), m_relais(relais) {}
  void init() override{};
  void update() override;

private:
  IRelais &m_relais;
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
  void setCursor(uint8_t col, uint8_t row) override{};
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
  void update(OutputDevicesIntent::RelaisCommand intent) override;

  OutputDevicesIntent::RelaisCommand recievedCommand();

private:
  OutputDevicesIntent::RelaisCommand lastCommand =
      OutputDevicesIntent::RelaisCommand::None;
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
  bool pressed() const override { return false; };
  bool released() override { return false; };
};
//}}}

class TestTemperatureSensorHardware : public ITempSensorHardware {
  //{{{
public:
  TestTemperatureSensorHardware() = default;
  void begin() override{};
  void requestTemperatures() override;
  float getTempCByIndex(uint8_t index = 0) override;
  void setTempReturn(float temp);
  void resetTestCounter();

  uint8_t requestTemperaturesCount = 0;
  uint8_t getTempCByIndexCount = 0;

private:
  float m_tempReturn = 0;
};
//}}}

#pragma GCC diagnostic pop
