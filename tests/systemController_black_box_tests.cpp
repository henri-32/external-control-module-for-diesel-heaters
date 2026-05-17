#include "ArduinoStubs.h"
#include "controller.h"
#include "test_devices.h"
#include "types.h"
#include <gtest/gtest.h>

// In these blackboxtests outputIntent.displayContent.status.state is
// used to check the state of the heater. This does not integrate logic
// from the display driver into these tests, because it's still a
// controller internal struct.
// Because the variable is directly set from the real controller state
// this way is prefered over loosing the encapsulation
// of the controller state.

using namespace ArduinoStubSpies;
using RelaisCmd = OutputDevicesIntent::RelaisCommand;

class SystemControllerBlackBox : public ::testing::Test {
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

TEST_F(SystemControllerBlackBox,
       relaisCommand_in_output_intent_resets_internally_after_tick_while_relais_recieved_command) {
//{{{
  inputData.switchAction.power = true;

  controller();
  EXPECT_EQ(relais.returnLastRecievedCommand(), RelaisCmd::Long);
  EXPECT_EQ(outputIntent.relaisCommand, RelaisCmd::None);
}
//}}}

TEST_F(
    SystemControllerBlackBox,
    power_switch_sends_relais_command_and_heater_state_switches_correctly) {
  //{{{
  inputData.switchAction.power = true;

  EXPECT_EQ(outputIntent.displayContent.status.state,
            HeaterStatus::HeatingState::OFF);
  EXPECT_EQ(relais.returnLastRecievedCommand(), RelaisCmd::None);

  controller();

  EXPECT_EQ(outputIntent.displayContent.status.state,
            HeaterStatus::HeatingState::ON);
  EXPECT_EQ(relais.returnLastRecievedCommand(), RelaisCmd::Long);
}
//}}}
