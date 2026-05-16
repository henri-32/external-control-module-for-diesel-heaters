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
    setMillis(config::temperatureRequestInterval);
    initSpies();
  }
};

TEST_F(TemperatureDriverTest, returns_hardware_request_value_when_interval_elapsed) {
  testSensor.setTempReturn(10.0);
  EXPECT_EQ(testSensor.getTempCByIndex(), 10.0);

  driver.pollTemp();
  advanceMillis(config::temperatureRequestInterval);

  EXPECT_EQ(driver.pollTemp(), 10.0);
}

TEST_F(TemperatureDriverTest, returns_previous_value_when_guard_interval_active) {
  testSensor.setTempReturn(10.0);
  driver.pollTemp();
  advanceMillis(config::temperatureRequestInterval);
  driver.pollTemp();

  testSensor.setTempReturn(20.0);
  EXPECT_EQ(driver.pollTemp(), 10.0);
}

TEST_F(TemperatureDriverTest,
       calls_request_but_not_read_when_conversion_time_not_elapsed) {
  driver.pollTemp();
  advanceMillis(700);
  driver.pollTemp();

  EXPECT_EQ(testSensor.requestTemperaturesCount, 1);
  EXPECT_EQ(testSensor.getTempCByIndexCount, 0);
}
