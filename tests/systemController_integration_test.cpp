#include "ArduinoStubs.h"
#include "controller.h"
#include "devicegroups.h"
#include "display_driver.h"
#include "encoder_driver.h"
#include "temperature_sensor_driver.h"
#include "test_devices.h"
#include <gtest/gtest.h>

using namespace ArduinoStubSpies;

class SystemControllerIntegrationTest : public ::testing::Test {
public:
  // Modifieable Hardwarestubs for Tests
  //=============================================================
  //=============================================================
  TestToggleSwitch powerSwitch;
  TestToggleSwitch modeSwitch;
  TestPushButton displayButton;
  TestEncoderHardware encoderHardware;
  TestTemperatureSensorHardware tempSensorHardware;
  TemperatureSensorDriver tempSensorDriver{tempSensorHardware};
  TestDisplayHardware displayHardware;
  TestRelais relais;
  //=============================================================
  //=============================================================

private:
  EncoderDriver encoderDriver{encoderHardware};
  DisplayDriver displayDriver{displayHardware, outputIntent.displayContent,
                              outputIntent.lcd_state};

  InputDevicesDataSet inputData;
  OutputDevicesIntent outputIntent;

  InputDevices inputDevices{inputData,     powerSwitch,   modeSwitch,
                            displayButton, encoderDriver, tempSensorDriver};

  OutputDevices outputDevices{outputIntent, displayDriver, relais};

  // Only for controller() operator use in testing to control the ticks
public:
  SystemController controller{inputDevices, outputDevices};

protected:
  void SetUp() override {
    controller.init();
    initSpies();
    setMillis(1000);
    controller();
  };
};

TEST_F(SystemControllerIntegrationTest, test) {
  powerSwitch.setNextChangedReturn(true);
  controller();
  EXPECT_EQ(relais.recievedCommand(), OutputDevicesIntent::RelaisCommand::Long);
}
