#include "ArduinoStubs.h"
#include "relaisdriver.h"
#include <gtest/gtest.h>

using namespace ArduinoStubSpies;
using COI = ControllerOutputIntent;

class RelaisDriverTest : public ::testing::Test {
protected:

  static constexpr uint8_t kPin = 7;
  RelaisDriver driver{kPin};

  void SetUp() override {
	resetSpies();
    setMillis(1000);
  }
};

TEST_F(RelaisDriverTest, init_configures_output_and_deactivates_relais) {
//{{{
  driver.init();

  EXPECT_EQ(pinWritten_mode, kPin);
  EXPECT_EQ(writtenMode, OUTPUT);
  EXPECT_EQ(pinWritten_state, kPin);
  EXPECT_EQ(writtenState, LOW);
}
//}}}

TEST_F(RelaisDriverTest, update_with_none_does_nothing_when_off) {
//{{{
  driver.update(COI::RelaisCommand::None);

  EXPECT_EQ(writtenState, NOSTATE);
  EXPECT_EQ(pinWritten_state, -1);
}
//}}}

TEST_F(RelaisDriverTest,
       short_command_activates_then_deactivates_on_next_update) {
//{{{
  driver.update(COI::RelaisCommand::Short);

  EXPECT_EQ(pinWritten_state, kPin);
  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(250);
  driver.update(COI::RelaisCommand::None);

  EXPECT_EQ(pinWritten_state, kPin);
  EXPECT_EQ(writtenState, LOW);
}
//{{{
TEST_F(RelaisDriverTest, long_command_stays_on_for_one_check_then_turns_off) {
//{{{
  driver.update(COI::RelaisCommand::Long);

  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(1000);
  driver.update(COI::RelaisCommand::None);
  EXPECT_EQ(writtenState, HIGH);

  advanceMillis(600);
  driver.update(COI::RelaisCommand::None);
  EXPECT_EQ(writtenState, LOW);
}
//}}}
