#include "controller.h"
#include "types.h"
#include <gtest/gtest.h>

class SystemControllerTest : public ::testing::Test {
protected:
  SystemController controller;
};

// Handling of Power Switch
//{{{
TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_OffToOn_NoRelayAction) {
  //{{{
  using State = HeaterStatus::HeatingState;
  controller.inputData.powerSwitchChanged = true;
  controller.inputData.alternatorPressed = true;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.heatingState = State::OFF;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState, State::ON)
      //{{{
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << " with Alternator";
  //}}}
  EXPECT_EQ(controller.inputData.alternatorUsed, true) << " with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_OnToOff_NoRelayAction) {
  //{{{
  using State = HeaterStatus::HeatingState;
  controller.inputData.powerSwitchChanged = true;
  controller.inputData.alternatorPressed = true;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.heatingState = State::ON;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState, State::OFF)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << " with Alternator";

  EXPECT_EQ(controller.inputData.alternatorUsed, true)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << " with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_UnrelatedInputIgnored) {
  //{{{
  using State = HeaterStatus::HeatingState;
  controller.inputData.powerSwitchChanged = true;
  controller.inputData.alternatorPressed = true;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.heatingState = State::OFF;

  // unrelated input
  controller.inputData.modeSwitchChanged = true;
  controller.inputData.encoder_val = 10;
  controller.inputData.alternatorReleased = true;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState, State::ON)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "with Alternator";

  EXPECT_EQ(controller.inputData.alternatorUsed, true) << "with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_UnrelatedInputIgnored_OnToOff) {
  //{{{
  using State = HeaterStatus::HeatingState;
  controller.inputData.powerSwitchChanged = true;
  controller.inputData.alternatorPressed = true;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.heatingState = State::ON;

  // unrelated input
  controller.inputData.modeSwitchChanged = true;
  controller.inputData.encoder_val = 10;
  controller.inputData.alternatorReleased = true;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState, State::OFF)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "with Alternator";

  EXPECT_EQ(controller.inputData.alternatorUsed, true)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithoutAlternator_OffAndRelayAction) {
  //{{{
  using State = HeaterStatus::HeatingState;
  using Command = ControllerOutputIntent::RelaisCommand;
  using Priority = ControllerOutputIntent::RelaisPriority;

  controller.inputData.powerSwitchChanged = true;
  controller.inputData.alternatorPressed = false;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.heatingState = State::ON;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState, State::OFF)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "without Alternator";

  EXPECT_EQ(controller.inputData.alternatorUsed, false)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "without Alternator";

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::POWER)
      << "state=" << static_cast<int>(controller.heaterStatus.mode)
      << "without Alternator";

  EXPECT_EQ(controller.outputIntent.m_relaisCommand, Command::Long)
      << "state=" << static_cast<int>(controller.outputIntent.m_relaisCommand)
      << "without Alternator";

  EXPECT_EQ(controller.outputIntent.m_currentPriority, Priority::High)
      << "state=" << static_cast<int>(controller.outputIntent.m_relaisCommand)
      << "without Alternator";
}
//}}}

//}}}

// Handling of Mode Switch Input
//{{{
TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWitchAlternator_POWERToTEMP_NoRelayAction) {
  //{{{
  controller.inputData.modeSwitchChanged = true;
  controller.inputData.alternatorPressed = true;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.mode = HeaterStatus::Mode::POWER;

  controller.applyModeSwitchInput();

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::TEMP)
      << "mode=" << static_cast<int>(controller.heaterStatus.mode)
      << "withAlternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWithAlternator_TEMPToPOWER_NoRelayAction) {
  //{{{
  controller.inputData.modeSwitchChanged = true;
  controller.inputData.alternatorPressed = true;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.mode = HeaterStatus::Mode::TEMP;

  controller.applyModeSwitchInput();

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::POWER)
      << "mode=" << static_cast<int>(controller.heaterStatus.mode)
      << "withAlternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWithoutAlternator_TEMPToPOWER) {
  //{{{
  controller.inputData.modeSwitchChanged = true;
  controller.inputData.alternatorPressed = false;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.mode = HeaterStatus::Mode::TEMP;

  controller.applyModeSwitchInput();

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::POWER)
      << "mode=" << static_cast<int>(controller.heaterStatus.mode)
      << "withAlternator";

  EXPECT_EQ(controller.outputIntent.m_relaisCommand,
            ControllerOutputIntent::RelaisCommand::Short)
      << "command=" << static_cast<int>(controller.outputIntent.m_relaisCommand)
      << " withoutAlternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWithoutAlternator_POWERToTEMP) {
  //{{{
  controller.inputData.modeSwitchChanged = true;
  controller.inputData.alternatorPressed = false;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.mode = HeaterStatus::Mode::POWER;
  controller.outputIntent.m_currentPriority =
      ControllerOutputIntent::RelaisPriority::Low;

  controller.applyModeSwitchInput();

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::TEMP)
      << "mode=" << static_cast<int>(controller.heaterStatus.mode)
      << "withAlternator";

  EXPECT_EQ(controller.outputIntent.m_relaisCommand,
            ControllerOutputIntent::RelaisCommand::Short)
      << "command=" << static_cast<int>(controller.outputIntent.m_relaisCommand)
      << " withoutAlternator";
}
//}}}

TEST_F(
    SystemControllerTest,
    applyModeSwitchInput_PathWithoutAlternator_POWERToTEMP_UnrelatedInputIgnored) {
  //{{{
  controller.inputData.modeSwitchChanged = true;
  controller.inputData.alternatorPressed = false;
  controller.inputData.alternatorUsed = false;
  controller.heaterStatus.mode = HeaterStatus::Mode::POWER;

  // unrelated Input
  controller.inputData.powerSwitchChanged = true;
  controller.inputData.encoder_val = 20;
  // should not make a difference, because get checked in seperate function
  controller.inputData.alternatorUsed = true;
  controller.inputData.alternatorReleased = true;

  controller.applyModeSwitchInput();

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::TEMP)
      << "mode=" << static_cast<int>(controller.heaterStatus.mode)
      << "withAlternator";

  EXPECT_EQ(controller.outputIntent.m_relaisCommand,
            ControllerOutputIntent::RelaisCommand::Short)
      << "command=" << static_cast<int>(controller.outputIntent.m_relaisCommand)
      << " withoutAlternator";
}

//}}}

//====================================
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
};
