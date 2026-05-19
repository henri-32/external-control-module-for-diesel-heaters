#include "ArduinoStubs.h"
#include "encoder_driver.h"
#include "test_devices.h"
#include <gtest/gtest.h>
using namespace ArduinoStubSpies;

// Test notes for EncoderDriver:
// - position simulates the absolute hardware encoder counter value.
// - The driver operates on deltas (current - prev) and counts only full
//   steps: 4 pulses == 1 step (steps = m_delta / 4).
// - Each TEST_F starts with a fresh fixture (m_prev=0, m_delta=0,
//   m_last_change_ms=0). There is no state transfer between tests.
// - After every position change, m_last_change_ms is reset.
//   As long as debounceConfig::encoder has not elapsed since the last
//   change, readSteps() always returns 0.
// - Changes are not discarded: they are accumulated in m_delta.
//   Only full steps are emitted; the remainder stays in delta.
//
// - The init() isn't really testable, because only internal states 
//   are manipulated. 

class EncoderDriverTest : public ::testing::Test {
protected:
  InputDevicesDataSet inputDataBuffer;
  TestEncoderHardware testEncoder;
  EncoderDriver driver{testEncoder};

  void SetUp() override {
    setMillis(0);
    testEncoder.position = 0;
    driver.readSteps();
  };
};

TEST_F(EncoderDriverTest, returns_zero_when_hardware_position_is_zero) {
  //{{{
  testEncoder.position = 0;
  EXPECT_EQ(driver.readSteps(), 0);
};
//}}}

TEST_F(EncoderDriverTest, returns_zero_within_debounce_interval) {
  //{{{
  // Within debounce
  testEncoder.position = 4;
  EXPECT_EQ(driver.readSteps(), 0);

  // Still within debounce
  advanceMillis(debounceConfig::encoder - 1);
  testEncoder.position = 4;
  EXPECT_EQ(driver.readSteps(), 0);
}
//}}}

TEST_F(EncoderDriverTest, returns_one_step_after_debounce_with_stable_position) {
  //{{{
  // First polling detects 0 -> 4, stores delta, and starts debounce.
  // Therefore no step is emitted yet.
  testEncoder.position = 4;
  driver.readSteps();

  // After debounce time has elapsed:
  // diff == 0, m_delta stays 4, emitted result 4/4 == 1.
  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest, carries_remainder_steps_into_later_reads) {
  //{{{

  testEncoder.position = 6;
  driver.readSteps();

  testEncoder.position = 6;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 1);

  testEncoder.position = 8;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 0);

  testEncoder.position = 8;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest,
       change_resets_debounce_even_when_previous_debounce_would_have_elapsed) {
  //{{{
  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 0);

  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 1);

  // Debounce since the last position change has not elapsed yet.
  testEncoder.position = 8;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder / 2);
  EXPECT_EQ(driver.readSteps(), 0);

  // The change to position 8 resets the debounce timer.
  advanceMillis(debounceConfig::encoder / 2);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest, repeated_reads_still_wait_until_debounce_after_change) {
  //{{{
  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 0);

  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 1);

  // Debounce since the last position change has not elapsed yet.
  testEncoder.position = 8;
  advanceMillis(debounceConfig::encoder / 2);
  EXPECT_EQ(driver.readSteps(), 0);

  // Even if the earlier quiet period (at position 4) would have elapsed:
  // The new change to position 8 restarted debounce.
  advanceMillis(debounceConfig::encoder / 2);
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder / 2);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest, returns_step_exactly_at_debounce_boundary) {
  //{{{
  testEncoder.position = 4;
  driver.readSteps();

  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder - 1);
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(1);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest, accumulates_multiple_full_steps_after_quiet_period) {
  //{{{
  testEncoder.position = 12;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 3);
};
//}}}

TEST_F(EncoderDriverTest, handles_negative_steps_and_remainder) {
  //{{{
  testEncoder.position = -6;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), -1);

  testEncoder.position = -8;
  EXPECT_EQ(driver.readSteps(), 0);
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), -1);
};
//}}}

TEST_F(EncoderDriverTest, no_output_when_no_new_movement_even_after_many_reads) {
  //{{{
  testEncoder.position = 4;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 1);

  for (int i = 0; i < 20; ++i) {
    advanceMillis(debounceConfig::encoder);
    EXPECT_EQ(driver.readSteps(), 0);
  }
};
//}}}

TEST_F(EncoderDriverTest, debounce_resets_on_every_intermediate_movement) {
  //{{{
  testEncoder.position = 4;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder / 3);
  testEncoder.position = 5;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder / 3);
  testEncoder.position = 8;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder / 3);
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 2);

  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 0);
};
//}}}

TEST_F(EncoderDriverTest, init_captures_initial_delta) {
  //{{{
  testEncoder.position = 100;
  driver.init();

  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 0);
};
//}}}

TEST_F(EncoderDriverTest, large_jump_translates_to_expected_step_count) {
  //{{{
  testEncoder.position = 400;
  driver.readSteps();

  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 100);
};
//}}}

TEST_F(EncoderDriverTest, back_and_forth_movement_cancels_delta) {
  //{{{
  testEncoder.position = 4;
  driver.readSteps();

  testEncoder.position = 0;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 0);
};
//}}}
