#include "config.h"
#include "controller.h"
#include "devicegroups.h"
#include "display_driver.h"
#include "encoder_driver.h"
#include "test_devices.h"
#include "temperature_sensor_driver.h"
#include <gtest/gtest.h>

class SystemControllerTest : public ::testing::Test {
protected:
  // Backing storage for device groups
  InputDevicesDataSet inputDataBuffer;
  OutputDevicesIntent outputIntentBuffer;

  // Hardware fakes
  TestDisplay testDisplay;
  TestRelais testRelais;
  TestToggleSwitch powerSwitch;
  TestToggleSwitch modeSwitch;
  TestPushButton displayButton;
  TestEncoderHardware encoderHardware;
  TestTemperatureSensorHardware tempSensorHardware;

  // Drivers
  DisplayDriver displayDriver{testDisplay, outputIntentBuffer.displayContent,
                              outputIntentBuffer.lcd_state};
  EncoderDriver encoderDriver{encoderHardware};
  TemperatureSensorDriver tempSensorDriver{tempSensorHardware};

  // Device groups
  InputDevices inputDevices{inputDataBuffer, powerSwitch, modeSwitch,
                            displayButton, encoderDriver, tempSensorDriver};
  OutputDevices outputDevices{outputIntentBuffer, displayDriver, testRelais};

  // System under test
  SystemController controller{inputDevices, outputDevices};

  // Shortcuts to controller-owned state
  InputDevicesDataSet &inputData = controller.inputDevices.data;
  OutputDevicesIntent &outputIntent = controller.outputDevices.intent;
};

// Handling of Power Switch Input
//{{{
TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_OffToOn_NoRelayAction) {
  //{{{
  using State = HeaterStatus::HeatingState;
  inputData.switchAction.power = true;
  inputData.alternator.pressed = true;
  inputData.alternator.used = false;
  controller.heaterStatus.heatingState = State::OFF;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState, State::ON)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << " with Alternator";
  EXPECT_EQ(inputData.alternator.used, true) << " with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_OnToOff_NoRelayAction) {
  //{{{
  using State = HeaterStatus::HeatingState;
  inputData.switchAction.power = true;
  inputData.alternator.pressed = true;
  inputData.alternator.used = false;
  controller.heaterStatus.heatingState = State::ON;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState, State::OFF)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << " with Alternator";

  EXPECT_EQ(inputData.alternator.used, true)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << " with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_UnrelatedInputIgnored) {
  //{{{
  using State = HeaterStatus::HeatingState;
  inputData.switchAction.power = true;
  inputData.alternator.pressed = true;
  inputData.alternator.used = false;
  controller.heaterStatus.heatingState = State::OFF;

  // unrelated input
  inputData.switchAction.mode = true;
  inputData.encoder_val = 10;
  inputData.alternator.released = true;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState, State::ON)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "with Alternator";

  EXPECT_EQ(inputData.alternator.used, true) << "with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithAlternator_UnrelatedInputIgnored_OnToOff) {
  //{{{
  using State = HeaterStatus::HeatingState;
  inputData.switchAction.power = true;
  inputData.alternator.pressed = true;
  inputData.alternator.used = false;
  controller.heaterStatus.heatingState = State::ON;

  // unrelated input
  inputData.switchAction.mode = true;
  inputData.encoder_val = 10;
  inputData.alternator.released = true;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState, State::OFF)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "with Alternator";

  EXPECT_EQ(inputData.alternator.used, true)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "with Alternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyPowerSwitchInput_PathWithoutAlternator_OffAndRelayAction) {
  //{{{
  using State = HeaterStatus::HeatingState;
  using Command = OutputDevicesIntent::RelaisCommand;
  using Priority = OutputDevicesIntent::RelaisPriority;

  inputData.switchAction.power = true;
  inputData.alternator.pressed = false;
  inputData.alternator.used = false;
  controller.heaterStatus.heatingState = State::ON;

  controller.applyPowerSwitchInput();

  EXPECT_EQ(controller.heaterStatus.heatingState, State::OFF)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "without Alternator";

  EXPECT_EQ(inputData.alternator.used, false)
      << "state=" << static_cast<int>(controller.heaterStatus.heatingState)
      << "without Alternator";

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::POWER)
      << "state=" << static_cast<int>(controller.heaterStatus.mode)
      << "without Alternator";

  EXPECT_EQ(outputIntent.m_relaisCommand, Command::Long)
      << "state=" << static_cast<int>(outputIntent.m_relaisCommand)
      << "without Alternator";

  EXPECT_EQ(outputIntent.m_currentPriority, Priority::High)
      << "state=" << static_cast<int>(outputIntent.m_relaisCommand)
      << "without Alternator";
}
//}}}

//}}}

// Handling of Mode Switch Input
//{{{
TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWitchAlternator_POWERToTEMP_NoRelayAction) {
  //{{{
  inputData.switchAction.mode = true;
  inputData.alternator.pressed = true;
  inputData.alternator.used = false;
  controller.heaterStatus.mode = HeaterStatus::Mode::POWER;

  controller.applyModeSwitchInput();

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::TEMP)
      << "mode=" << static_cast<int>(controller.heaterStatus.mode) << "withAlternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWithAlternator_TEMPToPOWER_NoRelayAction) {
  //{{{
  inputData.switchAction.mode = true;
  inputData.alternator.pressed = true;
  inputData.alternator.used = false;
  controller.heaterStatus.mode = HeaterStatus::Mode::TEMP;

  controller.applyModeSwitchInput();

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::POWER)
      << "mode=" << static_cast<int>(controller.heaterStatus.mode) << "withAlternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWithoutAlternator_TEMPToPOWER) {
  //{{{
  inputData.switchAction.mode = true;
  inputData.alternator.pressed = false;
  inputData.alternator.used = false;
  controller.heaterStatus.mode = HeaterStatus::Mode::TEMP;

  controller.applyModeSwitchInput();

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::POWER)
      << "mode=" << static_cast<int>(controller.heaterStatus.mode) << "withAlternator";

  EXPECT_EQ(outputIntent.m_relaisCommand,
            OutputDevicesIntent::RelaisCommand::Short)
      << "command=" << static_cast<int>(outputIntent.m_relaisCommand)
      << " withoutAlternator";
}
//}}}

TEST_F(SystemControllerTest,
       applyModeSwitchInput_PathWithoutAlternator_POWERToTEMP) {
  //{{{
  inputData.switchAction.mode = true;
  inputData.alternator.pressed = false;
  inputData.alternator.used = false;
  controller.heaterStatus.mode = HeaterStatus::Mode::POWER;
  outputIntent.m_currentPriority = OutputDevicesIntent::RelaisPriority::Low;

  controller.applyModeSwitchInput();

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::TEMP)
      << "mode=" << static_cast<int>(controller.heaterStatus.mode) << "withAlternator";

  EXPECT_EQ(outputIntent.m_relaisCommand,
            OutputDevicesIntent::RelaisCommand::Short)
      << "command=" << static_cast<int>(outputIntent.m_relaisCommand)
      << " withoutAlternator";
}
//}}}

TEST_F(
    SystemControllerTest,
    applyModeSwitchInput_PathWithoutAlternator_POWERToTEMP_UnrelatedInputIgnored) {
  //{{{
  inputData.switchAction.mode = true;
  inputData.alternator.pressed = false;
  inputData.alternator.used = false;
  controller.heaterStatus.mode = HeaterStatus::Mode::POWER;

  // unrelated Input
  inputData.switchAction.power = true;
  inputData.encoder_val = 20;
  // should not make a difference, because get checked in seperate function
  inputData.alternator.used = true;
  inputData.alternator.released = true;

  controller.applyModeSwitchInput();

  EXPECT_EQ(controller.heaterStatus.mode, HeaterStatus::Mode::TEMP)
      << "mode=" << static_cast<int>(controller.heaterStatus.mode) << "withAlternator";

  EXPECT_EQ(outputIntent.m_relaisCommand,
            OutputDevicesIntent::RelaisCommand::Short)
      << "command=" << static_cast<int>(outputIntent.m_relaisCommand)
      << " withoutAlternator";
}

//}}}
//}}}

// Handling of Display Button
//{{{
TEST_F(SystemControllerTest,
       applyDisplayButtonInput_noAction_without_Alternator) {
  //{{{
  inputData.alternator.released = true;
  inputData.alternator.pressed = true;
  inputData.alternator.used = true;
  outputIntent.lcd_state = OutputDevicesIntent::LCD_StateIntent::OFF;

  controller.applyDisplayButtonInput();

  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LCD_StateIntent::OFF);
  EXPECT_EQ(inputData.alternator.pressed, false);
  EXPECT_EQ(inputData.alternator.used, false);
}
//}}}

TEST_F(SystemControllerTest, applyDisplayButtonInput_outputIntent_sets) {
  //{{{
  inputData.alternator.pressed = false;
  inputData.alternator.used = false;
  inputData.alternator.released = true;
  outputIntent.lcd_state = OutputDevicesIntent::LCD_StateIntent::OFF;

  controller.applyDisplayButtonInput();

  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LCD_StateIntent::Page1);

  controller.applyDisplayButtonInput();
  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LCD_StateIntent::OFF);
}
//}}}

//}}}

// Handling of Encoder Input
//{{{
TEST_F(SystemControllerTest, applyEncoderInput_minStep) {
  //{{{
  using LCDIntent = OutputDevicesIntent::LCD_StateIntent;
  using LCDDirection = OutputDevicesIntent::LCD_CycleDirection;

  inputData.encoder_val = 1;
  inputData.alternator.pressed = true;
  outputIntent.lcd_state = LCDIntent::OFF;
  controller.heaterStatus.target_tempC = 10;

  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page1;

  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);
  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page2;

  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);
  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page3;

  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);
  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page4;

  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_maxStep) {
  //{{{
  using LCDIntent = OutputDevicesIntent::LCD_StateIntent;
  using LCDDirection = OutputDevicesIntent::LCD_CycleDirection;

  inputData.encoder_val = config::encoderValCutoff;
  inputData.alternator.pressed = true;
  outputIntent.lcd_state = LCDIntent::OFF;
  outputIntent.lcd_cycleDirection = LCDDirection::none;
  controller.heaterStatus.target_tempC = 10;

  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page1;

  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page2;
  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page3;
  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page4;
  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::right);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_negative_maxStep) {
  //{{{
  using LCDIntent = OutputDevicesIntent::LCD_StateIntent;
  using LCDDirection = OutputDevicesIntent::LCD_CycleDirection;

  inputData.encoder_val = -config::encoderValCutoff;
  inputData.alternator.pressed = true;
  outputIntent.lcd_state = LCDIntent::OFF;
  outputIntent.lcd_cycleDirection = LCDDirection::none;
  controller.heaterStatus.target_tempC = 10;

  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page4;

  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page3;
  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page2;
  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page1;
  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_negative_minStep) {
  //{{{
  /*Sollte beim Debuggen hier ein Problem auftauchen
   *siehe Zeile 250 */
  using LCDIntent = OutputDevicesIntent::LCD_StateIntent;
  using LCDDirection = OutputDevicesIntent::LCD_CycleDirection;

  inputData.encoder_val = -1;
  inputData.alternator.pressed = true;
  outputIntent.lcd_state = LCDIntent::OFF;
  outputIntent.lcd_cycleDirection = LCDDirection::none;
  controller.heaterStatus.target_tempC = 10;

  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page4;

  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page3;
  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page2;
  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::left);
  EXPECT_EQ(controller.heaterStatus.target_tempC, 10);

  outputIntent.lcd_cycleDirection = LCDDirection::none;
  outputIntent.lcd_state = LCDIntent::Page1;
  controller.applyEncoderInput();
  EXPECT_EQ(outputIntent.lcd_cycleDirection, LCDDirection::left);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_minStep_without_alternator) {
  //{{{
  float &target = controller.heaterStatus.target_tempC;
  inputData.encoder_val = 1;
  inputData.alternator.pressed = false;
  target = 10.0;

  controller.applyEncoderInput();

  EXPECT_EQ(target, 10.5);
}
//}}}

TEST_F(SystemControllerTest, applyEncoderInput_maxStep_without_alternator) {
  //{{{
  float &target = controller.heaterStatus.target_tempC;
  inputData.encoder_val = config::encoderValCutoff;
  inputData.alternator.pressed = false;
  target = 10.0;

  controller.applyEncoderInput();

  EXPECT_EQ(target, 10 + config::encoderValCutoff * config::tempStep);
}
//}}}

TEST_F(SystemControllerTest,
       applyEncoderInput_maxStep_without_alternator_over_guard) {
  //{{{
  float &target = controller.heaterStatus.target_tempC;
  inputData.encoder_val = config::encoderValCutoff;
  inputData.alternator.pressed = false;
  target = config::tempMax - 1;

  controller.applyEncoderInput();

  EXPECT_EQ(target, config::tempMax);
}
//}}}

TEST_F(SystemControllerTest,
       applyEncoderInput_negative_maxStep_without_alternator_below_guard) {
  //{{{
  float &target = controller.heaterStatus.target_tempC;
  inputData.encoder_val = -config::encoderValCutoff;
  inputData.alternator.pressed = false;
  target = config::tempMin + 1;

  controller.applyEncoderInput();

  EXPECT_EQ(target, config::tempMin);
}
//}}}

TEST_F(SystemControllerTest,
       applyEncoderInput_negative_min_Step_without_alternator) {
  //{{{
  float &target = controller.heaterStatus.target_tempC;
  inputData.encoder_val = -1;
  inputData.alternator.pressed = false;
  target = 10.0;

  controller.applyEncoderInput();

  EXPECT_EQ(target, 9.5);
}
//}}}

TEST_F(SystemControllerTest,
       applyEncoderInput_negative_max_Step_without_alternator) {
  //{{{
  float &target = controller.heaterStatus.target_tempC;
  inputData.encoder_val = -config::encoderValCutoff;
  inputData.alternator.pressed = false;
  target = 10.0;

  controller.applyEncoderInput();

  EXPECT_EQ(target, 10 - config::encoderValCutoff * config::tempStep);
}
//}}}
//}}}

// HELPER
//  applyHeatingLogic
//{{{
TEST_F(SystemControllerTest, applyHeatingLogic_too_cold_and_state_off) {
  //{{{
  using State = HeaterStatus::HeatingState;
  State &m_state = controller.heaterStatus.heatingState;

  inputData.sensor_tempC = 10;
  controller.heaterStatus.target_tempC = inputData.sensor_tempC + config::tolerance + 0.1;
  m_state = State::OFF;

  controller.applyHeatingLogic();

  EXPECT_EQ(m_state, State::ON);
  EXPECT_EQ(controller.outputDevices.intent.m_relaisCommand,
            OutputDevicesIntent::RelaisCommand::Long);
  EXPECT_EQ(controller.outputDevices.intent.m_currentPriority,
            OutputDevicesIntent::RelaisPriority::Low);
}
//}}}

TEST_F(SystemControllerTest, applyHeatingLogic_too_cold_and_state_on) {
  //{{{
  using State = HeaterStatus::HeatingState;
  State &m_state = controller.heaterStatus.heatingState;

  inputData.sensor_tempC = 10;
  controller.heaterStatus.target_tempC = inputData.sensor_tempC + config::tolerance + 0.1;
  m_state = State::ON;

  controller.applyHeatingLogic();

  EXPECT_EQ(m_state, State::ON);
}
//}}}

TEST_F(SystemControllerTest, applyHeatingLogic_too_warm_and_state_on) {
  //{{{
  using State = HeaterStatus::HeatingState;
  State &m_state = controller.heaterStatus.heatingState;

  inputData.sensor_tempC = 10;
  controller.heaterStatus.target_tempC = inputData.sensor_tempC - config::tolerance - 0.1;
  m_state = State::ON;

  controller.applyHeatingLogic();

  EXPECT_EQ(m_state, State::OFF);
  EXPECT_EQ(controller.outputDevices.intent.m_relaisCommand,
            OutputDevicesIntent::RelaisCommand::Long);
  EXPECT_EQ(controller.outputDevices.intent.m_currentPriority,
            OutputDevicesIntent::RelaisPriority::Low);
}
//}}}

TEST_F(SystemControllerTest, applyHeatingLogic_too_warm_and_state_off) {
  //{{{
  using State = HeaterStatus::HeatingState;
  State &m_state = controller.heaterStatus.heatingState;

  inputData.sensor_tempC = 10;
  controller.heaterStatus.target_tempC = inputData.sensor_tempC - config::tolerance - 0.1;
  m_state = State::OFF;

  controller.applyHeatingLogic();

  EXPECT_EQ(m_state, State::OFF);
}
//}}}

TEST_F(SystemControllerTest, applyHeatingLogic_early_return_by_wrong_mode) {
  //{{{
  using State = HeaterStatus::HeatingState;
  State &m_state = controller.heaterStatus.heatingState;
  controller.heaterStatus.mode = HeaterStatus::Mode::POWER;
  inputData.sensor_tempC = 10;
  controller.heaterStatus.target_tempC = inputData.sensor_tempC - config::tolerance - 0.1;
  m_state = State::ON;

  controller.applyHeatingLogic();

  EXPECT_EQ(m_state, State::ON);
  EXPECT_EQ(controller.outputDevices.intent.m_relaisCommand,
            OutputDevicesIntent::RelaisCommand::None);
  EXPECT_EQ(controller.outputDevices.intent.m_currentPriority,
            OutputDevicesIntent::RelaisPriority::Low);
}
//}}}
//}}}

// cyclePages
//{{{
TEST_F(SystemControllerTest, cyclePages_Intent_reacts_to_cycling_right) {
  //{{{
  using LCDIntent = OutputDevicesIntent::LCD_StateIntent;
  using LCDDirection = OutputDevicesIntent::LCD_CycleDirection;
  LCDIntent &lcdIntent = controller.outputDevices.intent.lcd_state;
  LCDDirection &lcdDirection = controller.outputDevices.intent.lcd_cycleDirection;

  lcdIntent = LCDIntent::Page1;
  lcdDirection = LCDDirection::right;

  controller.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page2);

  controller.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page3);

  controller.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page4);
}
//}}}

TEST_F(SystemControllerTest, cyclePages_Intent_reacts_to_cycling_left) {
  //{{{
  using LCDIntent = OutputDevicesIntent::LCD_StateIntent;
  using LCDDirection = OutputDevicesIntent::LCD_CycleDirection;
  LCDIntent &lcdIntent = controller.outputDevices.intent.lcd_state;
  LCDDirection &lcdDirection = controller.outputDevices.intent.lcd_cycleDirection;

  lcdIntent = LCDIntent::Page4;
  lcdDirection = LCDDirection::left;

  controller.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page3);

  controller.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page2);

  controller.cyclePages();

  EXPECT_EQ(lcdIntent, LCDIntent::Page1);
}
//}}}
//}}}

//====================================
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
};
