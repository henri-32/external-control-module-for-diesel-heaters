#include "controller.h"
#include "types.h"
#include <gtest/gtest.h>

class SystemControllerTest : public ::testing::Test {
protected:
  SystemController controller;
};

TEST_F(SystemControllerTest, applyPowerSwitchInputTest) {
  // Testing of Path, where with pressed Alternator no other Action than
  // Switching of the heatingState should occur.
  // From OFF to ON
  controller.inputData.powerSwitchChanged = true;
  controller.inputData.alternatorPressed = true;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.heatingState = HeaterStatus::HeatingState::OFF;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState,
            HeaterStatus::HeatingState::ON)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "From OFF to ON with Alternator";

  EXPECT_EQ(controller.inputData.alternatorUsed, true)
      << "From OFF to ON with Alternator";

  // From ON to OFF
  controller.inputData.powerSwitchChanged = true;
  controller.inputData.alternatorPressed = true;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.heatingState = HeaterStatus::HeatingState::ON;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState,
            HeaterStatus::HeatingState::OFF)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "From OFF to ON with Alternator";

  EXPECT_EQ(controller.inputData.alternatorUsed, true)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "From OFF to ON with Alternator";
};

//====================================
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
};
