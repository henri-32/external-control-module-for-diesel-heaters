#include "ArduinoStubs.h"
#include "encoder_driver.h"
#include "test_devices.h"
#include <gtest/gtest.h>
using namespace ArduinoStubSpies;

// Bei neuen Tests muss die testEncoder.nextTestRead Variable auf den
// "gemockten" Wert gesetzt werden
// Außerdem ist zu beachten, dass 4 Hardware Encoder Reads einem Driver 
// Read entsprechen sollten, da die Reads der Arduino Hardware nicht 
// relativiert werden. Dort werden bei einem physischen Klick 
// 4 Impulse gesendet. 

class EncoderDriverTest : public ::testing::Test {
protected:
  InputDevicesDataSet inputDataBuffer;
  TestEncoderHardware testEncoder;
  EncoderDriver driver{testEncoder};

  void SetUp() override { setMillis(0); };
};

TEST_F(EncoderDriverTest, return_from_driver_when_hardware_returns_0) {
  //{{{
  testEncoder.nextTestRead = 0;
  EXPECT_EQ(driver.readSteps(), 0);
};
//}}}

TEST_F(EncoderDriverTest, no_increase_in_debounce_interval) {
  //{{{
  //Innerhalb Debounce 
  testEncoder.nextTestRead = 4;
  EXPECT_EQ(driver.readSteps(), 0);

  //Immernoch innerhalb Debounce
  advanceMillis(debounceConfig::encoder - 1);
  testEncoder.nextTestRead = 4;
  EXPECT_EQ(driver.readSteps(), 0);
}
//}}}

TEST_F(EncoderDriverTest, increase_after_debounce_interval) {
  testEncoder.nextTestRead = 0; 

  advanceMillis(debounceConfig::encoder -1);
  testEncoder.nextTestRead = 4; 
  EXPECT_EQ(driver.readSteps(), 0); 

  // Außerhalb (genau auf) Debounce
  advanceMillis(1);
  EXPECT_EQ(driver.readSteps(), 1);
};

