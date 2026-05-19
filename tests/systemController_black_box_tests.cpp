#include "ArduinoStubs.h"
#include "controller.h"
#include "test_devices.h"
#include "types.h"
#include <gtest/gtest.h>

// NOTE:
// In these blackboxtests outputIntent.displayContent.status.state is
// used to check the state of the heater. This does not integrate logic
// from the display driver into these tests, because it's still a
// controller internal struct.
// Because the variable is directly set from the real controller state
// this way is prefered over loosing the encapsulation
// of the controller state.

// Because the switchActions do'nt get consumed in the tick, but instead
// are constantly read from the hardware they need to be set explixit for
// these isolatet tests.

using namespace ArduinoStubSpies;
using RelaisCmd = OutputDevicesIntent::RelaisCommand;

TEST(InitTests, controllerinit) {
  //{{{
  InputDevicesDataSet inputData;
  OutputDevicesIntent outputIntent;
  TestRelais relais;
  TestInputDevices testInput{inputData};
  TestOutputDevices testOutput{outputIntent, relais};
  SystemController controller{testInput, testOutput};

  controller.init();

  EXPECT_EQ(inputData.switchAction.mode, false);
  EXPECT_EQ(inputData.switchAction.power, false);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);
  EXPECT_EQ(outputIntent.relaisCommand, RelaisCmd::None);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::None);
  EXPECT_EQ(outputIntent.lcd_state, OutputDevicesIntent::LCD_StateIntent::OFF);
  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::TEMP);
  EXPECT_EQ(outputIntent.displayContent.status.target_tempC,
            Config::defaultTemp);
}
//}}}

class SystemControllerBlackBox : public ::testing::Test {
  //{{{
protected:
  InputDevicesDataSet inputData;
  OutputDevicesIntent outputIntent;

  // TestRelais is the only needed HardwareStub.
  // That's because there is no nice way to check the outgoing RelaisCommand
  // in the outputIntent. The Reason for that is that the command get's
  // consumed in controller() so outputIntent.relaisCommand is None after
  // every tick.

  TestRelais relais;
  TestInputDevices testInput{inputData};
  TestOutputDevices testOutput{outputIntent, relais};
  SystemController controller{testInput, testOutput};

  void SetUp() override {
    controller.init();
    inputData.sensor_tempC = 15.0;
    inputData.switchAction.mode = false;
    inputData.switchAction.power = false;
    inputData.encoder_val = 0;
    inputData.alternator.pressed = false;
    inputData.alternator.released = false;
    inputData.alternator.used = false;
    controller();
  };
};
//}}}

TEST_F(
    SystemControllerBlackBox,
    relaisCommand_in_output_intent_resets_internally_after_tick_while_relais_recieved_command) {
  //{{{
  inputData.switchAction.power = true;

  controller();
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Long);
  EXPECT_EQ(outputIntent.relaisCommand, RelaisCmd::None);
}
//}}}

TEST_F(
    SystemControllerBlackBox,
    power_switch_sends_relais_command_and_heater_state_switches_correctly_in_temp_mode) {
  //{{{
  inputData.switchAction.power = true;

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::None);

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::ON);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Long);
}
//}}}

TEST_F(
    SystemControllerBlackBox,
    mode_switch_send_relais_command_and_heater_mode_switches_correctly_from_temp_to_power_and_back) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::TEMP);
  inputData.switchAction.mode = true;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::POWER);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Short);

  inputData.switchAction.mode = true;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::TEMP);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Short);
}
//}}}

TEST_F(
    SystemControllerBlackBox,
    power_switch_changes_mode_from_temp_to_power_but_not_from_power_to_temp_while_sending_relais_command_and_switching_power_state) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::TEMP);
  inputData.switchAction.power = true;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::POWER);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::ON);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Long);

  inputData.switchAction.power = true;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::POWER);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Long);
}
//}}}

TEST_F(SystemControllerBlackBox, encoder_steps_change_target_temp) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.target_tempC,
            Config::defaultTemp);

  inputData.encoder_val = 1;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.target_tempC,
            Config::defaultTemp + Config::tempStep);
}
//}}}

TEST_F(SystemControllerBlackBox, encoder_negative_steps_change_target_temp) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.target_tempC,
            Config::defaultTemp);

  inputData.encoder_val = -1;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.target_tempC,
            Config::defaultTemp - Config::tempStep);
}
//}}}

TEST_F(SystemControllerBlackBox, display_button_turns_display_on_and_off) {
  //{{{
  EXPECT_EQ(outputIntent.lcd_state, OutputDevicesIntent::LCD_StateIntent::OFF);

  inputData.alternator.used = false;
  inputData.alternator.released = true;

  controller();

  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LCD_StateIntent::Page1);

  inputData.alternator.used = false;
  inputData.alternator.released = true;

  controller();

  EXPECT_EQ(outputIntent.lcd_state, OutputDevicesIntent::LCD_StateIntent::OFF);
}
//}}}

TEST_F(SystemControllerBlackBox,
       encoder_and_alternator_cycle_pages_and_consumes_alternator) {
  //{{{

  EXPECT_EQ(outputIntent.lcd_state, OutputDevicesIntent::LCD_StateIntent::OFF);

  inputData.alternator.used = false;
  inputData.alternator.released = true;

  controller();

  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LCD_StateIntent::Page1);

  inputData.alternator.released = false;
  inputData.alternator.pressed = true;
  inputData.alternator.used = false;
  inputData.encoder_val = 1;

  controller();

  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LCD_StateIntent::Page2);
  EXPECT_EQ(inputData.alternator.used, true);

  inputData.alternator.released = false;
  inputData.alternator.pressed = true;
  inputData.alternator.used = false;
  inputData.encoder_val = -1;

  controller();
  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LCD_StateIntent::Page1);
}
//}}}

TEST_F(SystemControllerBlackBox,
       modeSwitch_and_alternator_switches_only_state) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::TEMP);
  inputData.alternator.released = false;
  inputData.alternator.pressed = true;
  inputData.alternator.used = false;
  inputData.switchAction.mode = true;

  controller();

  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::None);
  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::POWER);
};
//}}}

TEST_F(SystemControllerBlackBox,
       powerSwitch_and_alternator_switches_only_state) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);

  inputData.alternator.released = false;
  inputData.alternator.pressed = true;
  inputData.alternator.used = false;
  inputData.switchAction.power = true;

  controller();

  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::None);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::ON);
};
//}}}

//
TEST_F(
    SystemControllerBlackBox,
    in_temp_mode_relais_command_and_state_switch_by_temperatures_out_of_tolerance_for_both_directions) {
  //{{{
  inputData.sensor_tempC = Config::defaultTemp - Config::tolerance;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::ON);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Long);

  inputData.sensor_tempC = Config::defaultTemp + Config::tolerance;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Long);
};
//}}}

TEST_F(
    SystemControllerBlackBox,
    no_actions_when_in_power_mode_with_temperatures_out_of_tolerance_for_both_directions) {
  //{{{

  inputData.switchAction.mode = true;
  controller();
  inputData.switchAction.mode = false;

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::POWER);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);

  inputData.sensor_tempC = Config::defaultTemp - Config::tolerance;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::POWER);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);

  inputData.sensor_tempC = Config::defaultTemp + Config::tolerance;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::POWER);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);
};
//}}}

TEST_F(
    SystemControllerBlackBox,
    no_relais_and_state_action_when_temp_is_out_of_tolerance_in_stable_state) {
  //{{{

  inputData.sensor_tempC = Config::defaultTemp - Config::tolerance;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::ON);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Long);

  controller();
  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::ON);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::None);

  inputData.sensor_tempC = Config::defaultTemp + Config::tolerance;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Long);

  controller();
  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::None);
};
//}}}

TEST_F(
    SystemControllerBlackBox,
    no_relais_and_state_action_when_temp_gets_into_tolerance_from_stable_state){
    //{{{

  inputData.sensor_tempC = Config::defaultTemp - Config::tolerance; 
  
  controller(); 

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::ON);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Long); 

  inputData.sensor_tempC = Config::defaultTemp - Config::tolerance + 1; 

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::ON);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::None); 

  inputData.sensor_tempC = Config::defaultTemp + Config::tolerance; 

  controller(); 
 
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::Long); 

  inputData.sensor_tempC = Config::defaultTemp + Config::tolerance - 1; 

  controller(); 

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::OFF);
  EXPECT_EQ(relais.recievedCommand(), RelaisCmd::None); 

  
};
//}}}
