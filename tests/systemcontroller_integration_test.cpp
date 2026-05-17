#include "controller.h"
#include "test_devices.h"
#include "types.h"
#include <gtest/gtest.h>

class SystemControllerIntegrationTest : public ::testing::Test {
protected:
  InputDevicesDataSet inputData; 
  OutputDevicesIntent outputIntent; 
  TestInputDevices testInput {inputData};
  TestOutputDevices testOutput{outputIntent};
  SystemController controller{testInput, testOutput};

  void SetUp() override {
    inputData.sensor_tempC = 15.0;
    inputData.switchAction.mode = false;
    inputData.switchAction.power = false;
    inputData.encoder_val = 0;
    inputData.alternator.pressed = false;
    inputData.alternator.released = false;
    inputData.alternator.used = false;

    controller.init();
  };
};

TEST_F(
    SystemControllerIntegrationTest,
    power_switch_switches_heater_on_when_targetTemp_and_measuredTemp_are_equal) {
  //{{{
    inputData.sensor_tempC = 15.0;
    inputData.switchAction.power = true;

  EXPECT_EQ(outputIntent.relaisCommand,
            OutputDevicesIntent::RelaisCommand::None);
  controller();
}
//}}}
