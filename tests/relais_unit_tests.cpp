#include "ArduinoStubs.h"
#include "relais.h"
#include <gtest/gtest.h>

using namespace ArduinoStubSpies;
using ODI = OutputDevicesIntent;


TEST(InitTests, relaisInit) {
//{{{
  Relais relais {8}; 

  relais.init();

  EXPECT_EQ(pinWritten_state, 8); 
  EXPECT_EQ(writtenState, LOW); 
  EXPECT_EQ(pinWritten_mode, 8); 
  EXPECT_EQ(writtenMode, OUTPUT); 
}; 

//}}}
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

TEST_F(RelaisTest, init_Configures_output_and_deactivates_relais) {
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

TEST_F(RelaisTest, short_command_activates_then_deactivates_on_next_update) {
  //{{{
  intentBuffer.relaisCommand = ODI::RelaisCommand::Short;
  driver.update(intentBuffer.relaisCommand);

  EXPECT_EQ(pinWritten_state, kPin);
  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(Config::kRelaisShortPulseMs);
  driver.update(ODI::RelaisCommand::None);

  EXPECT_EQ(pinWritten_state, kPin);
  EXPECT_EQ(writtenState, LOW);
}
//{{{

TEST_F(RelaisTest, long_command_activates_then_deactivates_on_next_update) {
  //{{{
  intentBuffer.relaisCommand = ODI::RelaisCommand::Short;
  driver.update(intentBuffer.relaisCommand);

  EXPECT_EQ(pinWritten_state, kPin);
  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(Config::kRelaisLongPulseMs);
  driver.update(ODI::RelaisCommand::None);

  EXPECT_EQ(pinWritten_state, kPin);
  EXPECT_EQ(writtenState, LOW);
}
//}}}

TEST_F(RelaisTest, short_command_stays_on_for_one_check_then_turns_off) {
  //{{{
  intentBuffer.relaisCommand = ODI::RelaisCommand::Short;
  driver.update(intentBuffer.relaisCommand);

  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(Config::kRelaisShortPulseMs - 1);
  driver.update(ODI::RelaisCommand::None);
  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(2);
  driver.update(ODI::RelaisCommand::None);
  EXPECT_EQ(writtenState, LOW);
}
//}}}
TEST_F(RelaisTest, long_command_stays_on_for_one_check_then_turns_off) {
  //{{{
  intentBuffer.relaisCommand = ODI::RelaisCommand::Long;
  driver.update(intentBuffer.relaisCommand);

  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(Config::kRelaisLongPulseMs - 1);
  driver.update(ODI::RelaisCommand::None);
  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(2);
  driver.update(ODI::RelaisCommand::None);
  EXPECT_EQ(writtenState, LOW);
}
//}}}
