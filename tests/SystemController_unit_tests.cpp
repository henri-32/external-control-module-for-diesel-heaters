#include "config.h"
#include "controller.h"
#include "types.h"
#include <gtest/gtest.h>

ControllerInputData inputData;
ControllerOutputIntent outputData;

TestInputDevices inputDevices{inputData};
TestOutputDevices outputDevices{outputData};

class SystemControllerTest : public ::testing::Test {
protected:
  SystemController c{inputDevices, outputDevices};
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
      << "state=" << static_cast<int>(c.heaterStatus.heatingState)
      << " with Alternator";
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

// Handling of Encoder Input
//{{{
TEST_F(SystemControllerTest, applyEncoderInput_minStep) {
  //{{{
  using LCDIntent = ControllerOutputIntent::LCD_StateIntent;
  using LCDDirection = ControllerOutputIntent::LCD_CycleDirection;

  input.encoder_val = 1;
  input.alternatorPressed = true;
  output.lcd_stateIntent = LCDIntent::OFF;
  c.heaterStatus.target_tempC = 10;

  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page1;

  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);
  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page2;

  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);
  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page3;

  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);
  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page4;

  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_maxStep) {
  //{{{
  using LCDIntent = ControllerOutputIntent::LCD_StateIntent;
  using LCDDirection = ControllerOutputIntent::LCD_CycleDirection;

  input.encoder_val = config::encoderValCutoff;
  input.alternatorPressed = true;
  output.lcd_stateIntent = LCDIntent::OFF;
  output.lcd_cycleDirection = LCDDirection::none;
  c.heaterStatus.target_tempC = 10;

  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page1;

  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page2;
  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page3;
  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page4;
  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_negative_maxStep) {
  //{{{
  using LCDIntent = ControllerOutputIntent::LCD_StateIntent;
  using LCDDirection = ControllerOutputIntent::LCD_CycleDirection;

  input.encoder_val = -config::encoderValCutoff;
  input.alternatorPressed = true;
  output.lcd_stateIntent = LCDIntent::OFF;
  output.lcd_cycleDirection = LCDDirection::none;
  c.heaterStatus.target_tempC = 10;

  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page4;

  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page3;
  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page2;
  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page1;
  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_negative_minStep) {
  //{{{
  /*Sollte beim Debuggen hier ein Problem auftauchen
   *siehe Zeile 250 */
  using LCDIntent = ControllerOutputIntent::LCD_StateIntent;
  using LCDDirection = ControllerOutputIntent::LCD_CycleDirection;

  input.encoder_val = -1;
  input.alternatorPressed = true;
  output.lcd_stateIntent = LCDIntent::OFF;
  output.lcd_cycleDirection = LCDDirection::none;
  c.heaterStatus.target_tempC = 10;

  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page4;

  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page3;
  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page2;
  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(c.heaterStatus.target_tempC, 10);

  output.lcd_cycleDirection = LCDDirection::none;
  output.lcd_stateIntent = LCDIntent::Page1;
  c.applyEncoderInput();
  EXPECT_EQ(output.lcd_cycleDirection, LCDDirection::left);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_minStep_without_alternator) {
  //{{{
  float &target = c.heaterStatus.target_tempC;
  input.encoder_val = 1;
  input.alternatorPressed = false;
  target = 10.0;

  c.applyEncoderInput();

  EXPECT_EQ(target, 10.5);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_maxStep_without_alternator) {
  //{{{
  float &target = c.heaterStatus.target_tempC;
  input.encoder_val = config::encoderValCutoff;
  input.alternatorPressed = false;
  target = 10.0;

  c.applyEncoderInput();

  EXPECT_EQ(target, 10 + config::encoderValCutoff * config::tempStep);
}
//}}}

TEST_F(SystemControllerTest,
       applyEncoderInput_maxStep_without_alternator_over_guard) {
  //{{{
  float &target = c.heaterStatus.target_tempC;
  input.encoder_val = config::encoderValCutoff;
  input.alternatorPressed = false;
  target = config::tempMax - 1;

  c.applyEncoderInput();

  EXPECT_EQ(target, config::tempMax);
}
//}}}

TEST_F(SystemControllerTest,
       applyEncoderInput_negative_maxStep_without_alternator_below_guard) {
  //{{{
  float &target = c.heaterStatus.target_tempC;
  input.encoder_val = -config::encoderValCutoff;
  input.alternatorPressed = false;
  target = config::tempMin + 1;

  c.applyEncoderInput();

  EXPECT_EQ(target, config::tempMin);
}
//}}}

TEST_F(SystemControllerTest,
       applyEncoderInput_negative_min_Step_without_alternator) {
  //{{{
  float &target = c.heaterStatus.target_tempC;
  input.encoder_val = -1;
  input.alternatorPressed = false;
  target = 10.0;

  c.applyEncoderInput();

  EXPECT_EQ(target, 9.5);
}
//}}}

TEST_F(SystemControllerTest,
       applyEncoderInput_negative_max_Step_without_alternator) {
  //{{{
  float &target = c.heaterStatus.target_tempC;
  input.encoder_val = -config::encoderValCutoff;
  input.alternatorPressed = false;
  target = 10.0;

  c.applyEncoderInput();

  EXPECT_EQ(target, 10 - config::encoderValCutoff * config::tempStep);
}
//}}}
//}}}

//HELPER
// applyHeatingLogic
//{{{
TEST_F(SystemControllerTest, applyHeatingLogic_too_cold_and_state_off) {
  //{{{
  using State = HeaterStatus::HeatingState;
  State &m_state = c.heaterStatus.heatingState;

  input.sensor_tempC = 10;
  c.heaterStatus.target_tempC = input.sensor_tempC + config::tolerance + 0.1;
  m_state = State::OFF;

  c.applyHeatingLogic();

  EXPECT_EQ(m_state, State::ON);
  EXPECT_EQ(c.outputIntent.m_relaisCommand,
            ControllerOutputIntent::RelaisCommand::Long);
  EXPECT_EQ(c.outputIntent.m_currentPriority,
            ControllerOutputIntent::RelaisPriority::Low);
}
//}}}

TEST_F(SystemControllerTest, applyHeatingLogic_too_cold_and_state_on) {
  //{{{
  using State = HeaterStatus::HeatingState;
  State &m_state = c.heaterStatus.heatingState;

  input.sensor_tempC = 10;
  c.heaterStatus.target_tempC = input.sensor_tempC + config::tolerance + 0.1;
  m_state = State::ON;

  c.applyHeatingLogic();

  EXPECT_EQ(m_state, State::ON);
}
//}}}

TEST_F(SystemControllerTest, applyHeatingLogic_too_warm_and_state_on) {
  //{{{
  using State = HeaterStatus::HeatingState;
  State &m_state = c.heaterStatus.heatingState;

  input.sensor_tempC = 10;
  c.heaterStatus.target_tempC = input.sensor_tempC - config::tolerance - 0.1;
  m_state = State::ON;

  c.applyHeatingLogic();

  EXPECT_EQ(m_state, State::OFF);
  EXPECT_EQ(c.outputIntent.m_relaisCommand,
            ControllerOutputIntent::RelaisCommand::Long);
  EXPECT_EQ(c.outputIntent.m_currentPriority,
            ControllerOutputIntent::RelaisPriority::Low);
}
//}}}

TEST_F(SystemControllerTest, applyHeatingLogic_too_warm_and_state_off) {
  //{{{
  using State = HeaterStatus::HeatingState;
  State &m_state = c.heaterStatus.heatingState;

  input.sensor_tempC = 10;
  c.heaterStatus.target_tempC = input.sensor_tempC - config::tolerance - 0.1;
  m_state = State::OFF;

  c.applyHeatingLogic();

  EXPECT_EQ(m_state, State::OFF);
}
//}}}

TEST_F(SystemControllerTest, applyHeatingLogic_early_return_by_wrong_mode) {
  //{{{
  using State = HeaterStatus::HeatingState;
  State &m_state = c.heaterStatus.heatingState;
  c.heaterStatus.mode = HeaterStatus::Mode::POWER;
  input.sensor_tempC = 10;
  c.heaterStatus.target_tempC = input.sensor_tempC - config::tolerance - 0.1;
  m_state = State::ON;

  c.applyHeatingLogic();

  EXPECT_EQ(m_state, State::ON);
  EXPECT_EQ(c.outputIntent.m_relaisCommand,
            ControllerOutputIntent::RelaisCommand::None);
  EXPECT_EQ(c.outputIntent.m_currentPriority,
            ControllerOutputIntent::RelaisPriority::Low);
}
//}}}
//}}}

// cyclePages
//{{{
TEST_F(SystemControllerTest, cyclePages_Intent_reacts_to_cycling_right) {
  //{{{
  using LCDIntent = ControllerOutputIntent::LCD_StateIntent;
  using LCDDirection = ControllerOutputIntent::LCD_CycleDirection;
  LCDIntent &lcdIntent = c.outputIntent.lcd_stateIntent;
  LCDDirection &lcdDirection = c.outputIntent.lcd_cycleDirection;

  lcdIntent = LCDIntent::Page1;
  lcdDirection = LCDDirection::right;

  c.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page2);

  c.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page3);

  c.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page4);
}
//}}}

TEST_F(SystemControllerTest, cyclePages_Intent_reacts_to_cycling_left) {
  //{{{
  using LCDIntent = ControllerOutputIntent::LCD_StateIntent;
  using LCDDirection = ControllerOutputIntent::LCD_CycleDirection;
  LCDIntent &lcdIntent = c.outputIntent.lcd_stateIntent;
  LCDDirection &lcdDirection = c.outputIntent.lcd_cycleDirection;

  lcdIntent = LCDIntent::Page4;
  lcdDirection = LCDDirection::left;

  c.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page3);

  c.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page2);

  c.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page1);
}
//}}}
//}}}

//====================================
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
};
