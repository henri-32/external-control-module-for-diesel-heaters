#include "ArduinoStubs.h"
#include "controller.h"
#include "test_devices.h"
#include "types.h"
#include <gtest/gtest.h>

// HINWEIS:
// In diesen Black-Box-Tests wird outputIntent.displayContent.status.state
// verwendet, um den Zustand der Heizung zu prüfen. Das integriert keine Logik
// aus dem Display-Treiber in diese Tests, weil es weiterhin ein
// controllerinternes Struct ist.
// Weil die Variable direkt aus dem echten Controller-Zustand gesetzt wird,
// ist dieser Weg dem Aufweichen der Kapselung des Controller-Zustands
// vorzuziehen.

// Weil die switchActions im Tick nicht konsumiert, sondern fortlaufend aus
// der Hardware gelesen werden, müssen sie für diese isolierten Tests
// explizit gesetzt werden.

using namespace ArduinoStubSpies;
using RelaisCmd = OutputDevicesIntent::RelaisCommand;

TEST(InitTests, controllerinit) {
  //{{{
  TestRuntimeConfig runtimeConfig;
  InputDevicesDataSet inputData;
  OutputDevicesIntent outputIntent;
  TestRelais relais;
  TestInputDevices testInput{inputData};
  TestOutputDevices testOutput{outputIntent, relais};
  SystemController controller{runtimeConfig, testInput, testOutput};

  controller.init();

  EXPECT_EQ(inputData.switchAction.mode, false);
  EXPECT_EQ(inputData.switchAction.power, false);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);
  EXPECT_EQ(outputIntent.relaisCommand, RelaisCmd::None);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
  EXPECT_EQ(outputIntent.lcd_state, OutputDevicesIntent::LcdStateIntent::Off);
  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Temp);
  EXPECT_EQ(outputIntent.displayContent.status.target_tempC,
            Config::kDefaultTempC);
}
//}}}

class SystemControllerBlackBox : public ::testing::Test {
  //{{{
protected:
  TestRuntimeConfig runtimeConfig;
  InputDevicesDataSet inputData;
  OutputDevicesIntent outputIntent;

  // TestRelais ist der einzige benötigte Hardware-Stub.
  // Der Grund ist, dass es keinen sauberen Weg gibt, den ausgehenden
  // RelaisCommand im outputIntent zu prüfen. Der Befehl wird in controller()
  // konsumiert, sodass outputIntent.relaisCommand nach jedem Tick None ist.

  TestRelais relais;
  TestInputDevices testInput{inputData};
  TestOutputDevices testOutput{outputIntent, relais};
  SystemController controller{runtimeConfig, testInput, testOutput};

  void SetUp() override {
    controller.init();
    inputData.sensor_tempC = 15.0;
    inputData.switchAction.mode = false;
    inputData.switchAction.power = false;
    inputData.encoder_val = 0;
    inputData.modifier.pressed = false;
    inputData.modifier.released = false;
    inputData.modifier.used = false;
    outputIntent.lcd_state = OutputDevicesIntent::LcdStateIntent::start_page;
    controller();
  };
};
//}}}

TEST_F(
    SystemControllerBlackBox,
    relaisCommand_in_output_intent_resets_internally_after_tick_while_relais_received_command) {
  //{{{
  inputData.switchAction.power = true;

  controller();
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);
  EXPECT_EQ(outputIntent.relaisCommand, RelaisCmd::None);
}
//}}}

TEST_F(
    SystemControllerBlackBox,
    power_switch_sends_relais_command_and_heater_state_switches_correctly_in_temp_mode) {
  //{{{
  inputData.switchAction.power = true;

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::On);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);
}
//}}}

TEST_F(
    SystemControllerBlackBox,
    mode_switch_send_relais_command_and_heater_mode_switches_correctly_from_temp_to_power_and_back) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Temp);
  inputData.switchAction.mode = true;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Power);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Short);

  inputData.switchAction.mode = true;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Temp);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Short);
}
//}}}

TEST_F(
    SystemControllerBlackBox,
    power_switch_changes_mode_from_temp_to_power_but_not_from_power_to_temp_while_sending_relais_command_and_switching_power_state) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Temp);
  inputData.switchAction.power = true;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Power);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::On);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  inputData.switchAction.power = true;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Power);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);
}
//}}}

TEST_F(SystemControllerBlackBox, encoder_steps_change_target_temp) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.target_tempC,
            Config::kDefaultTempC);

  inputData.encoder_val = 1;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.target_tempC,
            Config::kDefaultTempC + Config::kTempStepC);
}
//}}}

TEST_F(SystemControllerBlackBox, encoder_negative_steps_change_target_temp) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.target_tempC,
            Config::kDefaultTempC);

  inputData.encoder_val = -1;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.target_tempC,
            Config::kDefaultTempC - Config::kTempStepC);
}
//}}}

TEST_F(SystemControllerBlackBox, display_button_turns_display_on_and_off) {
  //{{{
  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LcdStateIntent::start_page);

  inputData.modifier.used = false;
  inputData.modifier.released = true;

  controller();

  EXPECT_EQ(outputIntent.lcd_state, OutputDevicesIntent::LcdStateIntent::Off);

  inputData.modifier.used = false;
  inputData.modifier.released = true;

  controller();

  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LcdStateIntent::start_page);
}
//}}}

TEST_F(SystemControllerBlackBox,
       encoder_and_modifier_cycle_pages_and_consumes_modifier) {
  //{{{

  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LcdStateIntent::start_page);

  outputIntent.lcd_state = OutputDevicesIntent::LcdStateIntent::Off;
  inputData.modifier.used = false;
  inputData.modifier.released = true;

  controller();

  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LcdStateIntent::start_page);

  inputData.modifier.released = false;
  inputData.modifier.pressed = true;
  inputData.modifier.used = false;
  inputData.encoder_val = 1;

  controller();

  EXPECT_EQ(outputIntent.lcd_state, OutputDevicesIntent::LcdStateIntent::Page2);
  EXPECT_EQ(inputData.modifier.used, true);

  inputData.modifier.released = false;
  inputData.modifier.pressed = true;
  inputData.modifier.used = false;
  inputData.encoder_val = -1;

  controller();
  EXPECT_EQ(outputIntent.lcd_state,
            OutputDevicesIntent::LcdStateIntent::start_page);
}
//}}}

TEST_F(SystemControllerBlackBox, modeSwitch_and_modifier_switches_only_state) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Temp);
  inputData.modifier.released = false;
  inputData.modifier.pressed = true;
  inputData.modifier.used = false;
  inputData.switchAction.mode = true;

  controller();

  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Power);
};
//}}}

TEST_F(SystemControllerBlackBox, powerSwitch_and_modifier_switches_only_state) {
  //{{{
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);

  inputData.modifier.released = false;
  inputData.modifier.pressed = true;
  inputData.modifier.used = false;
  inputData.switchAction.power = true;

  controller();

  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::On);
};
//}}}

TEST_F(
    SystemControllerBlackBox,
    in_temp_mode_relais_command_and_state_switch_by_temperatures_out_of_tolerance_for_both_directions) {
  //{{{
  inputData.sensor_tempC = Config::kDefaultTempC - Config::kToleranceC;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::On);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  inputData.sensor_tempC = Config::kDefaultTempC + Config::kToleranceC;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);
};
//}}}

TEST_F(
    SystemControllerBlackBox,
    no_actions_when_in_power_mode_with_temperatures_out_of_tolerance_for_both_directions) {
  //{{{

  inputData.switchAction.mode = true;
  controller();
  inputData.switchAction.mode = false;

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Power);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);

  inputData.sensor_tempC = Config::kDefaultTempC - Config::kToleranceC;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Power);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);

  inputData.sensor_tempC = Config::kDefaultTempC + Config::kToleranceC;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.mode, HeaterStatus::Mode::Power);
  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);
};
//}}}

TEST_F(
    SystemControllerBlackBox,
    no_relais_and_state_action_when_temp_is_out_of_tolerance_in_stable_state) {
  //{{{

  inputData.sensor_tempC = Config::kDefaultTempC - Config::kToleranceC;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::On);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  controller();
  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::On);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);

  inputData.sensor_tempC = Config::kDefaultTempC + Config::kToleranceC;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  controller();
  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
};
//}}}

TEST_F(
    SystemControllerBlackBox,
    no_relais_and_state_action_when_temp_gets_into_tolerance_from_stable_state) {
  //{{{

  inputData.sensor_tempC = Config::kDefaultTempC - Config::kToleranceC;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::On);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  inputData.sensor_tempC = Config::kDefaultTempC - Config::kToleranceC + 1;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::On);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);

  inputData.sensor_tempC = Config::kDefaultTempC + Config::kToleranceC;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  inputData.sensor_tempC = Config::kDefaultTempC + Config::kToleranceC - 1;

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state, HeaterStatus::State::Off);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
};
//}}}

TEST_F(
    SystemControllerBlackBox,
    default_temp_dialog_opens_changes_default_temp_and_sets_resets_accordingly) {
{{{
        EXPECT_EQ(outputIntent.lcd_state,
                  OutputDevicesIntent::LcdStateIntent::start_page);
        // sollte 4 Seiten nach rechts cyclen
        inputData.modifier.pressed = true;
        inputData.modifier.released = false;
        inputData.encoder_val = 1;
        controller();
        inputData.encoder_val = 1;
        controller();
        inputData.encoder_val = 1;
        controller();
        inputData.encoder_val = 1;
        controller();
        EXPECT_EQ(outputIntent.lcd_state,
                  OutputDevicesIntent::LcdStateIntent::default_temp_page);
        // Reset
        inputData.encoder_val = 0;
        inputData.modifier.pressed = false;
        inputData.modifier.released = false;

        // Drücken des Modusschalters um Dialog zu öffnen
        inputData.switchAction.mode = true;
        controller();
        inputData.switchAction.mode = false;
        EXPECT_EQ(outputIntent.lcd_state,
                  OutputDevicesIntent::LcdStateIntent::default_temp_dialog);

        // Encoder ändert intendet_default_target_tempC entsprechend
        EXPECT_EQ(controller.pendingDefaultTempC, 15.0);
        inputData.encoder_val = 4;
        controller();
        EXPECT_EQ(controller.pendingDefaultTempC,
                  15.0 + 4 * Config::kTempStepC);
        // Reset
        inputData.encoder_val = 0;

        // Drücken des Display/Modifier Schalters beendet den Dialog OHNE die
        // neue default_temp zu übernehmen
        inputData.modifier.released = true;
        inputData.modifier.used = false;
        controller();
        EXPECT_EQ(outputIntent.lcd_state,
                  OutputDevicesIntent::LcdStateIntent::default_temp_page);
        EXPECT_EQ(controller.runtimeConfig.get_default_tempC(), 15.0);
        // Reset
        inputData.modifier.released = false;

        // Erneutes Öffnen des Dialogs mit Verstellen des defaults.
        inputData.switchAction.mode = true;
        controller();
        inputData.switchAction.mode = false;
        EXPECT_EQ(outputIntent.lcd_state,
                  OutputDevicesIntent::LcdStateIntent::default_temp_dialog);
        EXPECT_EQ(controller.runtimeConfig.get_default_tempC(), 15.0);
        inputData.encoder_val = 4;
        controller();
        EXPECT_EQ(controller.pendingDefaultTempC,
                  15.0 + 4 * Config::kTempStepC);
        // Reset
        inputData.encoder_val = 0;

        // Drücken des Mode Schalters ÜBERNIMMT die neue default_temp
        inputData.switchAction.mode = true;
        controller();
        EXPECT_EQ(controller.runtimeConfig.get_default_tempC(),
                  15.0 + 4 * Config::kTempStepC);

        // Reset
        inputData.switchAction.mode = false;
}
}}}
