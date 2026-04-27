#include "test_devices.h"

TestDisplay::TestDisplay() = default;
void TestDisplay::cyclePages(
    ControllerOutputIntent::LCD_CycleDirection direction) {
  m_lastGivenDirection = direction;
};

TestInputDevices::TestInputDevices(ControllerInputData &id) { (void)id; }

TestOutputDevices::TestOutputDevices(ControllerOutputIntent &oi) { (void)oi; }
