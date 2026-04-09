#include "units_for_testing.h"
#include <cstring>

extern "C" {
#include "unity.h"
}

void setUp(void) {
  heaterStatus = HeaterStatus{};
  inputData = ControllerInputData{};
  outputIntent = ControllerOutputIntent{};
}
void tearDown(void) {}

// TESTCASES============================================================================
void test_applyModeSwitchInput() {
  inputData.modeSwitchChanged = true;
  heaterStatus.mode = HeaterStatus::Mode::POWER;

  applyModeSwitchInput();
  HeaterStatus::Mode state_1 = heaterStatus.mode;
  ControllerOutputIntent::RelaisCommand command_1 = outputIntent.relaisCommand;
  applyModeSwitchInput();
  HeaterStatus::Mode state_2 = heaterStatus.mode;
  ControllerOutputIntent::RelaisCommand command_2 = outputIntent.relaisCommand;

  TEST_ASSERT_TRUE(state_1 == HeaterStatus::Mode::TEMP &&
                   command_1 == ControllerOutputIntent::RelaisCommand::Short &&
                   state_2 == HeaterStatus::Mode::POWER &&
                   command_2 == ControllerOutputIntent::RelaisCommand::Short);
}

void test_applyPowerSwitchInput() {
  inputData.powerSwitchChanged = true;
  heaterStatus.heatingState = HeaterStatus::HeatingState::ON;

  applyPowerSwitchInput();
  HeaterStatus::HeatingState state_1 = heaterStatus.heatingState;
  ControllerOutputIntent::RelaisCommand command_1 = outputIntent.relaisCommand;
  applyPowerSwitchInput();
  HeaterStatus::HeatingState state_2 = heaterStatus.heatingState;
  ControllerOutputIntent::RelaisCommand command_2 = outputIntent.relaisCommand;

  TEST_ASSERT_TRUE(state_1 == HeaterStatus::HeatingState::OFF &&
                   command_1 == ControllerOutputIntent::RelaisCommand::Long &&
                   state_2 == HeaterStatus::HeatingState::ON &&
                   command_2 == ControllerOutputIntent::RelaisCommand::Long);
}


// RUNNER LOGIK
// ==========================================================================
int main(int argc, char **argv) {
  const char *selected = (argc > 1) ? argv[1] : nullptr;

#define TEST_CASE(name)                                                        \
  if (selected == nullptr || std::strcmp(selected, #name) == 0) {              \
    RUN_TEST(name);                                                            \
  }
#include "test_cases.def"
#undef TEST_CASE

  return UNITY_END();
}
