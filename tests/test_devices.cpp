#include "test_devices.h"

TestInputDevices::TestInputDevices(ControllerInputData &id) { (void)id; }

TestOutputDevices::TestOutputDevices(ControllerOutputIntent &oi,
                                     IDisplay &display) : m_outputIntent(oi), m_display(display) {}
