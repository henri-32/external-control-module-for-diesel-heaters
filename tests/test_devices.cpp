#include "test_devices.h"

TestDisplay::TestDisplay() = default;
void TestDisplay::cyclePages(
    ControllerOutputIntent::LCD_CycleDirection direction) {
  if (direction == ControllerOutputIntent::LCD_CycleDirection::right) {
    switch (m_displayState) {
    case intent::OFF:
      return;
    case intent::Page1:
      m_displayState = intent::Page2;
      break;
    case intent::Page2:
      m_displayState = intent::Page3;
      break;
    case intent::Page3:
      m_displayState = intent::Page4;
      break;
    case intent::Page4:
      m_displayState = intent::Page1;
      break;
    };
  };

  if (direction == ControllerOutputIntent::LCD_CycleDirection::left) {
    switch (m_displayState) {
    case intent::OFF:
      return;
    case intent::Page1:
      m_displayState = intent::Page3;
      break;
    case intent::Page2:
      m_displayState = intent::Page1;
      break;
    case intent::Page3:
      m_displayState = intent::Page2;
      break;
    case intent::Page4:
      m_displayState = intent::Page3;
      break;
    }
    return;
  }
}
TestInputDevices::TestInputDevices(ControllerInputData &id){};

TestOutputDevices::TestOutputDevices(ControllerOutputIntent &oi){};
