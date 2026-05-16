#include "ArduinoStubs.h"
#include "temperature_sensor_driver.h"
#include "test_devices.h"
#include <gtest/gtest.h>

using namespace ArduinoStubSpies;

class TemperatureDriverTest : public ::testing::Test {
protected:
  TestTemperatureSensorHardware testSensor;
  TemperatureSensorDriver driver{testSensor};

  void SetUp() override {
    setMillis(0);
    initSpies();
  }
};

TEST_F(TemperatureDriverTest, request_of_hardware_returns_correctly) {
  testSensor.setTempReturn(10.0);
  EXPECT_EQ(testSensor.getTempCByIndex(), 10.0);
  
  advanceMillis(config::temperatureRequestInterval); 
  driver.pollTemp(); 
  advanceMillis(config::temperatureRequestInterval); 

  EXPECT_EQ(driver.pollTemp(), 10.0);
}
