#include "interfaces.h"
#include "types.h"

class TestDisplay {
public:
  TestDisplay();
  void cyclePages(ControllerOutputIntent::LCD_CycleDirection ci) {}
};

class TestInputDevices : public InputDevices {
public:
  TestInputDevices(ControllerInputData &id);
  void init() override {}
  void updateInputData() override {}
};

class TestOutputDevices : public OutputDevices {
public:
  TestOutputDevices(ControllerOutputIntent &oi);
  void init() override {}
  void update() override {}

  TestDisplay m_lcdDisplay{};
};
