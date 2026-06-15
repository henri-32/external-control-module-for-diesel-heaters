#pragma once
#include "interfaces.h"
#include <vector> 
#include <string> 
#include <utility> 
#include <vector> 
#include <string> 
#include <utility> 

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

class TestInputDevices : public IInputDevices {
public:
  TestInputDevices(InputDevicesDataSet &inputData) : IInputDevices(inputData) {}
  //{{{
// Diese Klasse dient dazu, den Controller isoliert zu testen,
// indem sie für Tests direkten Zugriff auf inputData und outputIntent ermöglicht.
// Die reale Klasse InputDevices arbeitet mit Interfaces, sodass es möglich ist,
// die reale Klasse zusammen mit Hardware-Stubs zu verwenden. Das geschieht im
// Integrationstest des SystemController, erfordert jedoch, dass alle HardwareStubs
// initialisiert werden, und verhindert den direkten Zugriff auf die I/O-Structs.
// Diese Klasse ermöglicht das isolierte Testen des SystemControllers über dessen Ein-/
// Ausgabe-Structs

  void init() override{};
  void update() override{};
};
//}}}

class TestOutputDevices : public IOutputDevices {
  //{{{
  // Siehe Kommentar TestInputDevices
public:
  TestOutputDevices(OutputDevicesIntent &outputIntent, IRelais &relais)
      : IOutputDevices(outputIntent), m_relais(relais) {}
  void init() override{};
  void update() override;

private:
  IRelais &m_relais;
};
//}}}

//==================================================================
// Die folgenden Klassen sind die jeweiligen Test-Stubs zu den Geräten
//==================================================================

class TestDisplayHardware : public IDisplayHardware {
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

  OutputDevicesIntent::RelaisCommand receivedCommand();
  void reset(); 

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
  bool changed() override { return nextReturn; };
  void setNextChangedReturn(bool nr);

private: 
  bool nextReturn = false; 
};
//}}}

class TestPushButton : public IPushButton {
  //{{{
public:
  TestPushButton() = default;
  void init() override{};
  bool pressed() const override { return nextPressed; };
  bool released() override { return nextReleased; };
  void setNextPressedReturn(bool nr);
  void setNextReleasedReturn(bool nr);

private:
  bool nextPressed = false;
  bool nextReleased = false;
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
