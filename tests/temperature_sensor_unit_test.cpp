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
    setMillis(Config::kTemperatureRequestIntervalMs);
    initSpies();
    testSensor.setTempReturn(10.0);
  }
};

TEST_F(TemperatureDriverTest,
       returns_hardware_request_value_when_interval_elapsed) {
  //{{{
  testSensor.setTempReturn(10.0);
  EXPECT_EQ(testSensor.getTempCByIndex(), 10.0);

  driver.pollTemp();
  advanceMillis(Config::kTemperatureRequestIntervalMs);

  EXPECT_EQ(driver.pollTemp(), 10.0);
}
//}}}

TEST_F(TemperatureDriverTest,
       returns_previous_value_when_guard_interval_active) {
  //{{{
  testSensor.setTempReturn(10.0);
  driver.pollTemp();
  advanceMillis(Config::kTemperatureRequestIntervalMs);
  driver.pollTemp();
  advanceMillis(Config::kTemperatureRequestIntervalMs - 1);

  testSensor.setTempReturn(20.0);
  EXPECT_EQ(driver.pollTemp(), 10.0);
}
//}}}

TEST_F(TemperatureDriverTest,
       returns_correct_value_when_guard_interval_passed_exactly) {
  //{{{
  testSensor.setTempReturn(10.0);
  driver.pollTemp();
  advanceMillis(Config::kTemperatureRequestIntervalMs);
  EXPECT_EQ(driver.pollTemp(), 10.0);

  advanceMillis(Config::kTemperatureRequestIntervalMs);
  testSensor.setTempReturn(20.0);
  driver.pollTemp();
  advanceMillis(Config::kTemperatureRequestIntervalMs);
  EXPECT_EQ(driver.pollTemp(), 20.0);
}
//}}}

TEST_F(TemperatureDriverTest,
       calls_request_but_not_read_when_conversion_time_not_elapsed) {
  //{{{
  driver.pollTemp();
  advanceMillis(749);
  driver.pollTemp();

  EXPECT_EQ(testSensor.requestTemperaturesCount, 1);
  EXPECT_EQ(testSensor.getTempCByIndexCount, 0);
}
//}}}

TEST_F(TemperatureDriverTest, correct_read_on_conversion_time) {
  //{{{
  driver.pollTemp();
  advanceMillis(750);
  driver.pollTemp();

  EXPECT_EQ(testSensor.requestTemperaturesCount, 1);
  EXPECT_EQ(testSensor.getTempCByIndexCount, 1);
}
//}}}

TEST_F(TemperatureDriverTest, no_double_requests_while_request_pending) {
  //{{{
  driver.pollTemp();
  advanceMillis(Config::kTemperatureRequestIntervalMs);
  driver.pollTemp();
  advanceMillis(10);
  driver.pollTemp();
  EXPECT_EQ(testSensor.requestTemperaturesCount, 2);
}
//}}}
