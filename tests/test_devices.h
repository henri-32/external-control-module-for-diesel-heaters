#include "interfaces.h"
#include "types.h"

class TestDisplay {
public:
  TestDisplay();
  using LCDDirection = ControllerOutputIntent::LCD_CycleDirection; 
  using LCDIntent = ControllerOutputIntent::LCD_StateIntent; 

  LCDDirection m_lastGivenDirection = LCDDirection::none;
  LCDIntent m_stateIntent = LCDIntent::OFF;

  void cyclePages(LCDDirection direction);

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
