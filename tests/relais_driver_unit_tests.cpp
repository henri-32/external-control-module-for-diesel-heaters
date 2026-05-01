#include "relaisdriver.h"
#include "ArduinoStubs.h"
#include <gtest/gtest.h>

class RelaisDriverTest : public ::testing::Test {
protected:
  static constexpr uint8_t kPin = 7;
  RelaisDriver driver{kPin};

  void SetUp() override {
    ArduinoStub::Spies::pinWritten_state = -1; // Sentinel: kein digitalWrite-Aufruf
    ArduinoStub::Spies::writtenState = LOW;
    ArduinoStub::Spies::pinWritten_mode = -1; // Sentinel: kein pinMode-Aufruf
    ArduinoStub::Spies::writtenMode = INPUT;
  }
};

TEST_F(RelaisDriverTest, init_configures_output_and_deactivates_relais) {
  driver.init();

  EXPECT_EQ(ArduinoStub::Spies::pinWritten_mode, kPin);
  EXPECT_EQ(ArduinoStub::Spies::writtenMode, OUTPUT);
  EXPECT_EQ(ArduinoStub::Spies::pinWritten_state, kPin);
  EXPECT_EQ(ArduinoStub::Spies::writtenState, LOW);
}

TEST_F(RelaisDriverTest, update_with_none_does_nothing_when_off) {
  driver.update(ControllerOutputIntent::RelaisCommand::None);

  EXPECT_EQ(ArduinoStub::Spies::pinWritten_state, -1);
}

TEST_F(RelaisDriverTest, short_command_activates_then_deactivates_on_next_update) {
  driver.update(ControllerOutputIntent::RelaisCommand::Short);

  EXPECT_EQ(ArduinoStub::Spies::pinWritten_state, kPin);
  EXPECT_EQ(ArduinoStub::Spies::writtenState, HIGH);

  driver.update(ControllerOutputIntent::RelaisCommand::None);

  EXPECT_EQ(ArduinoStub::Spies::pinWritten_state, kPin);
  EXPECT_EQ(ArduinoStub::Spies::writtenState, LOW);
}

TEST_F(RelaisDriverTest, long_command_stays_on_for_one_check_then_turns_off) {
  driver.update(ControllerOutputIntent::RelaisCommand::Long);

  EXPECT_EQ(ArduinoStub::Spies::writtenState, HIGH);

  driver.update(ControllerOutputIntent::RelaisCommand::None);
  EXPECT_EQ(ArduinoStub::Spies::writtenState, HIGH);

  driver.update(ControllerOutputIntent::RelaisCommand::None);
  EXPECT_EQ(ArduinoStub::Spies::writtenState, LOW);
}
