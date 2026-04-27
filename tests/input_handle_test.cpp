#include "controller.h"
#include "types.h"
#include "config.h"
#include <gtest/gtest.h>

class SystemControllerTest : public ::testing::Test {
protected:
  SystemController c;
  ControllerInputData &input = c.inputData;
  ControllerOutputIntent &output = c.outputIntent;
};

// Handling of Power Switch Input
//{{{
TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_OffToOn_NoRelayAction) {
  //{{{
  using State = HeaterStatus::HeatingState;
  input.powerSwitchChanged = true;
  input.alternatorPressed = true;
  input.alternatorUsed = false;
  c.heaterStatus.heatingState = State::OFF;

  c.applyPowerSwitchInput();

  EXPECT_EQ(c.heaterStatus.heatingState, State::ON)
      //{{{
      << "state=" << static_cast<int>(c.heaterStatus.heatingState)
      << " with Alternator";
  //}}}
  EXPECT_EQ(input.alternatorUsed, true) << " with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_OnToOff_NoRelayAction) {
  //{{{
  using State = HeaterStatus::HeatingState;
  input.powerSwitchChanged = true;
  input.alternatorPressed = true;
  input.alternatorUsed = false;
  c.heaterStatus.heatingState = State::ON;

  c.applyPowerSwitchInput();

  EXPECT_EQ(c.heaterStatus.heatingState, State::OFF)
      << "state=" << static_cast<int>(c.heaterStatus.heatingState)
      << " with Alternator";

  EXPECT_EQ(input.alternatorUsed, true)
      << "state=" << static_cast<int>(c.heaterStatus.heatingState)
      << " with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_UnrelatedInputIgnored) {
  //{{{
  using State = HeaterStatus::HeatingState;
  input.powerSwitchChanged = true;
  input.alternatorPressed = true;
  input.alternatorUsed = false;
  c.heaterStatus.heatingState = State::OFF;

  // unrelated input
  input.modeSwitchChanged = true;
  input.encoder_val = 10;
  input.alternatorReleased = true;

  c.applyPowerSwitchInput();

  EXPECT_EQ(c.heaterStatus.heatingState, State::ON)
      << "state=" << static_cast<int>(c.heaterStatus.heatingState)
      << "with Alternator";

  EXPECT_EQ(input.alternatorUsed, true) << "with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_UnrelatedInputIgnored_OnToOff) {
  //{{{
  using State = HeaterStatus::HeatingState;
  input.powerSwitchChanged = true;
  input.alternatorPressed = true;
  input.alternatorUsed = false;
  c.heaterStatus.heatingState = State::ON;

  // unrelated input
  input.modeSwitchChanged = true;
  input.encoder_val = 10;
  input.alternatorReleased = true;

  c.applyPowerSwitchInput();

  EXPECT_EQ(c.heaterStatus.heatingState, State::OFF)
      << "state=" << static_cast<int>(c.heaterStatus.heatingState)
      << "with Alternator";

  EXPECT_EQ(input.alternatorUsed, true)
      << "state=" << static_cast<int>(c.heaterStatus.heatingState)
      << "with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithoutAlternator_OffAndRelayAction) {
  //{{{
  using State = HeaterStatus::HeatingState;
  using Command = ControllerOutputIntent::RelaisCommand;
  using Priority = ControllerOutputIntent::RelaisPriority;

  input.powerSwitchChanged = true;
  input.alternatorPressed = false;
  input.alternatorUsed = false;
  c.heaterStatus.heatingState = State::ON;

  c.applyPowerSwitchInput();

  EXPECT_EQ(c.heaterStatus.heatingState, State::OFF)
      << "state=" << static_cast<int>(c.heaterStatus.heatingState)
      << "without Alternator";

  EXPECT_EQ(input.alternatorUsed, false)
      << "state=" << static_cast<int>(c.heaterStatus.heatingState)
      << "without Alternator";

  EXPECT_EQ(c.heaterStatus.mode, HeaterStatus::Mode::POWER)
      << "state=" << static_cast<int>(c.heaterStatus.mode)
      << "without Alternator";

  EXPECT_EQ(output.m_relaisCommand, Command::Long)
      << "state=" << static_cast<int>(output.m_relaisCommand)
      << "without Alternator";

  EXPECT_EQ(output.m_currentPriority, Priority::High)
      << "state=" << static_cast<int>(output.m_relaisCommand)
      << "without Alternator";
}
//}}}

//}}}

// Handling of Mode Switch Input
//{{{
TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWitchAlternator_POWERToTEMP_NoRelayAction) {
  //{{{
  input.modeSwitchChanged = true;
  input.alternatorPressed = true;
  input.alternatorUsed = false;
  c.heaterStatus.mode = HeaterStatus::Mode::POWER;

  c.applyModeSwitchInput();

  EXPECT_EQ(c.heaterStatus.mode, HeaterStatus::Mode::TEMP)
      << "mode=" << static_cast<int>(c.heaterStatus.mode) << "withAlternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWithAlternator_TEMPToPOWER_NoRelayAction) {
  //{{{
  input.modeSwitchChanged = true;
  input.alternatorPressed = true;
  input.alternatorUsed = false;
  c.heaterStatus.mode = HeaterStatus::Mode::TEMP;

  c.applyModeSwitchInput();

  EXPECT_EQ(c.heaterStatus.mode, HeaterStatus::Mode::POWER)
      << "mode=" << static_cast<int>(c.heaterStatus.mode) << "withAlternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWithoutAlternator_TEMPToPOWER) {
  //{{{
  input.modeSwitchChanged = true;
  input.alternatorPressed = false;
  input.alternatorUsed = false;
  c.heaterStatus.mode = HeaterStatus::Mode::TEMP;

  c.applyModeSwitchInput();

  EXPECT_EQ(c.heaterStatus.mode, HeaterStatus::Mode::POWER)
      << "mode=" << static_cast<int>(c.heaterStatus.mode) << "withAlternator";

  EXPECT_EQ(output.m_relaisCommand,
            ControllerOutputIntent::RelaisCommand::Short)
      << "command=" << static_cast<int>(output.m_relaisCommand)
      << " withoutAlternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWithoutAlternator_POWERToTEMP) {
  //{{{
  input.modeSwitchChanged = true;
  input.alternatorPressed = false;
  input.alternatorUsed = false;
  c.heaterStatus.mode = HeaterStatus::Mode::POWER;
  output.m_currentPriority = ControllerOutputIntent::RelaisPriority::Low;

  c.applyModeSwitchInput();

  EXPECT_EQ(c.heaterStatus.mode, HeaterStatus::Mode::TEMP)
      << "mode=" << static_cast<int>(c.heaterStatus.mode) << "withAlternator";

  EXPECT_EQ(output.m_relaisCommand,
            ControllerOutputIntent::RelaisCommand::Short)
      << "command=" << static_cast<int>(output.m_relaisCommand)
      << " withoutAlternator";
}
//}}}

TEST_F(
    SystemControllerTest,
    applyModeSwitchInput_PathWithoutAlternator_POWERToTEMP_UnrelatedInputIgnored) {
  //{{{
  input.modeSwitchChanged = true;
  input.alternatorPressed = false;
  input.alternatorUsed = false;
  c.heaterStatus.mode = HeaterStatus::Mode::POWER;

  // unrelated Input
  input.powerSwitchChanged = true;
  input.encoder_val = 20;
  // should not make a difference, because get checked in seperate function
  input.alternatorUsed = true;
  input.alternatorReleased = true;

  c.applyModeSwitchInput();

  EXPECT_EQ(c.heaterStatus.mode, HeaterStatus::Mode::TEMP)
      << "mode=" << static_cast<int>(c.heaterStatus.mode) << "withAlternator";

  EXPECT_EQ(output.m_relaisCommand,
            ControllerOutputIntent::RelaisCommand::Short)
      << "command=" << static_cast<int>(output.m_relaisCommand)
      << " withoutAlternator";
}

//}}}
//}}}

// Handling of Encoder Input
//{{{
TEST_F(SystemControllerTest, applyEncoderInput_minStep) {
  //{{{
  using LCDIntent = ControllerOutputIntent::LCD_StateIntent;
  using LCDDirection = ControllerOutputIntent::LCD_CycleDirection;
  LCDDirection& ldir =  c.outputDevices.m_lcdDisplay.m_lastGivenDirection;

  input.encoder_val = 1;
  input.alternatorPressed = true;
  output.lcd_stateIntent = LCDIntent::OFF;
  ldir = LCDDirection::none;

  c.applyEncoderInput();
  EXPECT_EQ(c.outputDevices.m_lcdDisplay.m_lastGivenDirection,LCDDirection::right);

  ldir = LCDDirection::none; 
  output.lcd_stateIntent = LCDIntent::Page1;

  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::right);
  ldir = LCDDirection::none; 
  output.lcd_stateIntent = LCDIntent::Page2;

  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::right);
  ldir = LCDDirection::none; 
  output.lcd_stateIntent = LCDIntent::Page3;

  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::right);
  ldir = LCDDirection::none; 
  output.lcd_stateIntent = LCDIntent::Page4;

  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::right);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_maxStep) {
  //{{{
  /*Sollte beim Debuggen hier ein Problem auftauchen
   *siehe Zeile 250 */
  using LCDIntent = ControllerOutputIntent::LCD_StateIntent;
  using LCDDirection = ControllerOutputIntent::LCD_CycleDirection;
  LCDDirection &ldir = c.outputDevices.m_lcdDisplay.m_lastGivenDirection;

  input.encoder_val = guards::encoderValCutoff;
  input.alternatorPressed = true;
  output.lcd_stateIntent = LCDIntent::OFF;
  ldir = LCDDirection::none;

  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::right);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page1;

  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::right);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page2;
  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::right);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page3;
  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::right);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page4;
  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::right);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_negative_maxStep) {
  //{{{
  /*Sollte beim Debuggen hier ein Problem auftauchen
   *siehe Zeile 250 */
  using LCDIntent = ControllerOutputIntent::LCD_StateIntent;
  using LCDDirection = ControllerOutputIntent::LCD_CycleDirection;
  LCDDirection &ldir = c.outputDevices.m_lcdDisplay.m_lastGivenDirection;

  input.encoder_val = -guards::encoderValCutoff;
  input.alternatorPressed = true;
  output.lcd_stateIntent = LCDIntent::OFF;
  ldir = LCDDirection::none;

  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::left);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page4;

  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::left);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page3;
  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::left);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page2;
  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::left);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page1;
  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::left);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_negative_minStep) {
  //{{{
  /*Sollte beim Debuggen hier ein Problem auftauchen
   *siehe Zeile 250 */
  using LCDIntent = ControllerOutputIntent::LCD_StateIntent;
  using LCDDirection = ControllerOutputIntent::LCD_CycleDirection;
  LCDDirection &ldir = c.outputDevices.m_lcdDisplay.m_lastGivenDirection;

  input.encoder_val = -1;
  input.alternatorPressed = true;
  output.lcd_stateIntent = LCDIntent::OFF;
  ldir = LCDDirection::none;

  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::left);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page4;

  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::left);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page3;
  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::left);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page2;
  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::left);

  ldir = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page1;
  c.applyEncoderInput();
  EXPECT_EQ(ldir, LCDDirection::left);
}
//}}}

//}}}

// Handling of Display Button
//{{{
TEST_F(SystemControllerTest,
       applyDisplayButtonInput_noAction_without_Alternator) {
  //{{{
  input.alternatorReleased = true;
  input.alternatorPressed = true;
  input.alternatorUsed = true;
  output.lcd_stateIntent = ControllerOutputIntent::LCD_StateIntent::OFF;

  c.applyDisplayButtonInput();

  EXPECT_EQ(output.lcd_stateIntent,
            ControllerOutputIntent::LCD_StateIntent::OFF);
  EXPECT_EQ(input.alternatorPressed, false);
  EXPECT_EQ(input.alternatorUsed, false);
}
//}}}

TEST_F(SystemControllerTest, applyDisplayButtonInput_outputIntent_sets) {
  //{{{
  input.alternatorPressed = false;
  input.alternatorUsed = false;
  input.alternatorReleased = true;
  output.lcd_stateIntent = ControllerOutputIntent::LCD_StateIntent::OFF;

  c.applyDisplayButtonInput();

  EXPECT_EQ(output.lcd_stateIntent,
            ControllerOutputIntent::LCD_StateIntent::Page1);

  c.applyDisplayButtonInput();
  EXPECT_EQ(output.lcd_stateIntent,
            ControllerOutputIntent::LCD_StateIntent::OFF);
}
//}}}

//}}}

//====================================
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
};
