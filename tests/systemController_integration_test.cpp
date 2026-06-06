#include "ArduinoStubs.h"
#include "controller.h"
#include "devicegroups.h"
#include "display_driver.h"
#include "encoder_driver.h"
#include "temperature_sensor_driver.h"
#include "test_devices.h"
#include <gtest/gtest.h>
#include <string>

// Naming is equivalent to the SystemController black box Tests.
// Assertions use observable test-stub outputs: relais and display hardware.

using namespace ArduinoStubSpies;
using RelaisCmd = OutputDevicesIntent::RelaisCommand;

class SystemControllerIntegrationTest : public ::testing::Test {
  //{{{
public:
  // Modifiable Hardwarestubs for Tests
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

public:
  // Only for controller() operator use in testing to control the ticks
  SystemController controller{inputDevices, outputDevices};

protected:
  void SetUp() override {
    initSpies();
    setMillis(1000);
    tempSensorHardware.setTempReturn(15.0F);
    controller.init();
    controller();
    relais.reset();
  };

  void pulsePowerSwitch() {
    advanceMillis(100);
    powerSwitch.setNextChangedReturn(true);
    controller();
    powerSwitch.setNextChangedReturn(false);
  }

  void pulseModeSwitch() {
    advanceMillis(100);
    modeSwitch.setNextChangedReturn(true);
    controller();
    modeSwitch.setNextChangedReturn(false);
  }

  void pressDisplayButton() { displayButton.setNextPressedReturn(true); }

  void releaseDisplayButton() {
    displayButton.setNextPressedReturn(false);
    displayButton.setNextReleasedReturn(true);
    controller();
    displayButton.setNextReleasedReturn(false);
  }

  void rotateEncoderSteps(int steps) {
    encoderHardware.position += steps * 4;
    controller();
    advanceMillis(DebounceConfig::kEncoderMs + 1);
    controller();
  }

  void pollSensorTemp(float temp) {
    tempSensorHardware.setTempReturn(temp);
    advanceMillis(Config::kTemperatureRequestIntervalMs);
    controller();
    advanceMillis(750);
    controller();
  }

  std::string latestDisplayLineStartingWith(const std::string &prefix) const {
    for (auto it = displayHardware.printed_lines.rbegin();
         it != displayHardware.printed_lines.rend(); ++it) {
      if (it->rfind(prefix, 0) == 0) {
        return *it;
      }
    }
    return "";
  }

  void expectDisplayState(const char *state) const {
    EXPECT_EQ(latestDisplayLineStartingWith("Zustand:"), state);
  }

  void expectDisplayMode(const char *mode) const {
    EXPECT_EQ(latestDisplayLineStartingWith("Mode:"), mode);
  }

  void expectDisplayTargetTemp(const char *temp) const {
    EXPECT_EQ(latestDisplayLineStartingWith("Solltemp.:") , std::string(temp));
  }
};
//}}}

TEST_F(SystemControllerIntegrationTest, controllerinit) {
  //{{{
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
  EXPECT_EQ(displayHardware.no_backlight_calls, 2);
  EXPECT_EQ(displayHardware.no_display_calls, 2);

  releaseDisplayButton();

  expectDisplayState("Zustand:   OFF");
  expectDisplayMode("Mode:      TEMP");
  expectDisplayTargetTemp("Solltemp.: 15.0 C");
}
//}}}

TEST_F(
    SystemControllerIntegrationTest,
    relaisCommand_in_output_intent_resets_internally_after_tick_while_relais_received_command) {
  //{{{
  pulsePowerSwitch();

  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
}
//}}}

TEST_F(
    SystemControllerIntegrationTest,
    power_switch_sends_relais_command_and_heater_state_switches_correctly_in_temp_mode) {
  //{{{
  releaseDisplayButton();
  relais.reset();

  expectDisplayState("Zustand:   OFF");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);

  pulsePowerSwitch();

  expectDisplayState("Zustand:   ON");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);
}
//}}}

TEST_F(
    SystemControllerIntegrationTest,
    mode_switch_send_relais_command_and_heater_mode_switches_correctly_from_temp_to_power_and_back) {
  //{{{
  releaseDisplayButton();
  relais.reset();

  expectDisplayMode("Mode:      TEMP");

  pulseModeSwitch();

  expectDisplayMode("Mode:      POWER");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Short);

  pulseModeSwitch();

  expectDisplayMode("Mode:      TEMP");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Short);
}
//}}}

TEST_F(
    SystemControllerIntegrationTest,
    power_switch_changes_mode_from_temp_to_power_but_not_from_power_to_temp_while_sending_relais_command_and_switching_power_state) {
  //{{{
  releaseDisplayButton();
  relais.reset();

  expectDisplayMode("Mode:      TEMP");

  pulsePowerSwitch();

  expectDisplayMode("Mode:      POWER");
  expectDisplayState("Zustand:   ON");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  pulsePowerSwitch();

  expectDisplayMode("Mode:      POWER");
  expectDisplayState("Zustand:   OFF");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);
}
//}}}

TEST_F(SystemControllerIntegrationTest, encoder_steps_change_target_temp) {
  //{{{
  releaseDisplayButton();
  relais.reset();

  expectDisplayTargetTemp("Solltemp.: 15.0 C");

  rotateEncoderSteps(1);

  expectDisplayTargetTemp("Solltemp.: 15.5 C");
}
//}}}

TEST_F(SystemControllerIntegrationTest, encoder_negative_steps_change_target_temp) {
  //{{{
  releaseDisplayButton();
  relais.reset();

  expectDisplayTargetTemp("Solltemp.: 15.0 C");

  rotateEncoderSteps(-1);

  expectDisplayTargetTemp("Solltemp.: 14.5 C");
}
//}}}

TEST_F(SystemControllerIntegrationTest, display_button_turns_display_on_and_off) {
  //{{{
  const int no_display_calls_before = displayHardware.no_display_calls;

  releaseDisplayButton();

  EXPECT_GT(displayHardware.display_calls, 0);

  releaseDisplayButton();

  EXPECT_GT(displayHardware.no_display_calls, no_display_calls_before);
}
//}}}

TEST_F(SystemControllerIntegrationTest,
       encoder_and_alternator_cycle_pages_and_consumes_alternator) {
  //{{{
  releaseDisplayButton();
  relais.reset();
  const int display_calls_on_page1 = displayHardware.display_calls;

  pressDisplayButton();
  rotateEncoderSteps(1);

  EXPECT_GT(displayHardware.display_calls, display_calls_on_page1);

  const int no_display_calls_before_release = displayHardware.no_display_calls;
  releaseDisplayButton();

  EXPECT_EQ(displayHardware.no_display_calls, no_display_calls_before_release);

  pressDisplayButton();
  rotateEncoderSteps(-1);

  expectDisplayState("Zustand:   OFF");
}
//}}}

TEST_F(SystemControllerIntegrationTest,
       modeSwitch_and_alternator_switches_only_state) {
  //{{{
  releaseDisplayButton();
  relais.reset();

  expectDisplayMode("Mode:      TEMP");

  pressDisplayButton();
  modeSwitch.setNextChangedReturn(true);
  advanceMillis(100);

  controller();

  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
  expectDisplayMode("Mode:      POWER");
}
//}}}

TEST_F(SystemControllerIntegrationTest,
       powerSwitch_and_alternator_switches_only_state) {
  //{{{
  releaseDisplayButton();
  relais.reset();

  expectDisplayState("Zustand:   OFF");

  pressDisplayButton();
  powerSwitch.setNextChangedReturn(true);
  advanceMillis(100);

  controller();

  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
  expectDisplayState("Zustand:   ON");
}
//}}}

TEST_F(
    SystemControllerIntegrationTest,
    in_temp_mode_relais_command_and_state_switch_by_temperatures_out_of_tolerance_for_both_directions) {
  //{{{
  releaseDisplayButton();
  relais.reset();

  pollSensorTemp(Config::kDefaultTempC - Config::kToleranceC);

  expectDisplayState("Zustand:   ON");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  pollSensorTemp(Config::kDefaultTempC + Config::kToleranceC);

  expectDisplayState("Zustand:   OFF");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);
}
//}}}

TEST_F(
    SystemControllerIntegrationTest,
    no_actions_when_in_power_mode_with_temperatures_out_of_tolerance_for_both_directions) {
  //{{{
  releaseDisplayButton();
  relais.reset();

  pulseModeSwitch();
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Short);

  expectDisplayMode("Mode:      POWER");
  expectDisplayState("Zustand:   OFF");

  pollSensorTemp(Config::kDefaultTempC - Config::kToleranceC);

  expectDisplayMode("Mode:      POWER");
  expectDisplayState("Zustand:   OFF");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);

  pollSensorTemp(Config::kDefaultTempC + Config::kToleranceC);

  expectDisplayMode("Mode:      POWER");
  expectDisplayState("Zustand:   OFF");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
}
//}}}

TEST_F(
    SystemControllerIntegrationTest,
    no_relais_and_state_action_when_temp_is_out_of_tolerance_in_stable_state) {
  //{{{
  releaseDisplayButton();
  relais.reset();

  pollSensorTemp(Config::kDefaultTempC - Config::kToleranceC);

  expectDisplayState("Zustand:   ON");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  controller();
  controller();

  expectDisplayState("Zustand:   ON");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);

  pollSensorTemp(Config::kDefaultTempC + Config::kToleranceC);

  expectDisplayState("Zustand:   OFF");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  controller();
  controller();

  expectDisplayState("Zustand:   OFF");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
}
//}}}

TEST_F(
    SystemControllerIntegrationTest,
    no_relais_and_state_action_when_temp_gets_into_tolerance_from_stable_state) {
  //{{{
  releaseDisplayButton();
  relais.reset();

  pollSensorTemp(Config::kDefaultTempC - Config::kToleranceC);

  expectDisplayState("Zustand:   ON");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  pollSensorTemp(Config::kDefaultTempC - Config::kToleranceC + 1);

  expectDisplayState("Zustand:   ON");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);

  pollSensorTemp(Config::kDefaultTempC + Config::kToleranceC);

  expectDisplayState("Zustand:   OFF");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::Long);

  pollSensorTemp(Config::kDefaultTempC + Config::kToleranceC - 1);

  expectDisplayState("Zustand:   OFF");
  EXPECT_EQ(relais.receivedCommand(), RelaisCmd::None);
}
//}}}
