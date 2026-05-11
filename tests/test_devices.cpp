#include "test_devices.h"

void TestDisplay::init() {
m_init_called = true;}

int TestEncoderHardware::read() {
  return nextTestRead; 
}
