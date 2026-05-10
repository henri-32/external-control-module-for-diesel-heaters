#include "ArduinoStubs.h"
#include "relais.h"
#include <gtest/gtest.h>

using namespace ArduinoStubSpies;
using ODI = OutputDevicesIntent;

class RelaisTest : public ::testing::Test {
protected:

  static constexpr uint8_t kPin = 7;
  ODI intentBuffer;
  Relais driver{kPin};

  void SetUp() override {
	initSpies();
    setMillis(1000);
  }
};

TEST_F(RelaisTest, init_configures_output_and_deactivates_relais) {
//{{{
  driver.init();

  EXPECT_EQ(pinWritten_mode, kPin);
  EXPECT_EQ(writtenMode, OUTPUT);
  EXPECT_EQ(pinWritten_state, kPin);
  EXPECT_EQ(writtenState, LOW);
}
//}}}

TEST_F(RelaisTest, update_with_none_does_nothing_when_off) {
//{{{
  driver.update(intentBuffer.relaisCommand);

  EXPECT_EQ(writtenState, NOSTATE);
  EXPECT_EQ(pinWritten_state, -1);
}
//}}}

TEST_F(RelaisTest,
       short_command_activates_then_deactivates_on_next_update) {
//{{{
  intentBuffer.relaisCommand = ODI::RelaisCommand::Short; 
  driver.update(intentBuffer.relaisCommand);

  EXPECT_EQ(pinWritten_state, kPin);
  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(250);
  driver.update(ODI::RelaisCommand::None);

  EXPECT_EQ(pinWritten_state, kPin);
  EXPECT_EQ(writtenState, LOW);
}
//{{{
TEST_F(RelaisTest, long_command_stays_on_for_one_check_then_turns_off) {
//{{{
  intentBuffer.relaisCommand = ODI::RelaisCommand::Long; 
  driver.update(intentBuffer.relaisCommand);

  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(1000);
  driver.update(ODI::RelaisCommand::None);
  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(600);
  driver.update(ODI::RelaisCommand::None);
  EXPECT_EQ(writtenState, LOW);
}
//}}}
