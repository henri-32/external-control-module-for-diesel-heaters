#include "ArduinoStubs.h"
#include "encoder_driver.h"
#include "test_devices.h"
#include <gtest/gtest.h>
using namespace ArduinoStubSpies;

// Testhinweise für EncoderDriver:
// - position simuliert den absoluten Hardware-Zählerwert des Encoders.
// - Der Treiber arbeitet mit Deltas (current - prev) und zählt nur ganze
//   Schritte: 4 Impulse == 1 Schritt (steps = m_delta / 4).
// - Jeder TEST_F startet mit einer frischen Fixture (m_prev=0, m_delta=0,
//   m_last_change_ms=0). Es gibt keine Zustandsübernahme zwischen Tests.
// - Nach jeder Positionsänderung wird m_last_change_ms zurückgesetzt.
//   Solange DebounceConfig::kEncoderMs seit der letzten Änderung nicht
//   vergangen ist, gibt readSteps() immer 0 zurück.
// - Änderungen werden nicht verworfen: Sie werden in m_delta gesammelt.
//   Nur ganze Schritte werden ausgegeben; der Rest bleibt in delta.
//
// - init() ist nicht wirklich testbar, weil nur interne Zustände
//   manipuliert werden.

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
  // Innerhalb der Entprellzeit
  testEncoder.position = 4;
  EXPECT_EQ(driver.readSteps(), 0);

  // Immer noch innerhalb der Entprellzeit
  advanceMillis(DebounceConfig::kEncoderMs - 1);
  testEncoder.position = 4;
  EXPECT_EQ(driver.readSteps(), 0);
}
//}}}

TEST_F(EncoderDriverTest, returns_one_step_after_debounce_with_stable_position) {
  //{{{
  // Die erste Abfrage erkennt 0 -> 4, speichert delta und startet die Entprellung.
  // Deshalb wird noch kein Schritt ausgegeben.
  testEncoder.position = 4;
  driver.readSteps();

  // Nachdem die Entprellzeit abgelaufen ist:
  // diff == 0, m_delta bleibt 4, ausgegebenes Ergebnis 4/4 == 1.
  testEncoder.position = 4;
  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest, carries_remainder_steps_into_later_reads) {
  //{{{

  testEncoder.position = 6;
  driver.readSteps();

  testEncoder.position = 6;
  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 1);

  testEncoder.position = 8;
  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 0);

  testEncoder.position = 8;
  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest,
       change_resets_debounce_even_when_previous_debounce_would_have_elapsed) {
  //{{{
  testEncoder.position = 4;
  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 0);

  testEncoder.position = 4;
  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 1);

  // Die Entprellzeit seit der letzten Positionsänderung ist noch nicht abgelaufen.
  testEncoder.position = 8;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(DebounceConfig::kEncoderMs / 2);
  EXPECT_EQ(driver.readSteps(), 0);

  // Die Änderung auf Position 8 setzt den Entprell-Timer zurück.
  advanceMillis(DebounceConfig::kEncoderMs / 2);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest, repeated_reads_still_wait_until_debounce_after_change) {
  //{{{
  testEncoder.position = 4;
  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 0);

  testEncoder.position = 4;
  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 1);

  // Die Entprellzeit seit der letzten Positionsänderung ist noch nicht abgelaufen.
  testEncoder.position = 8;
  advanceMillis(DebounceConfig::kEncoderMs / 2);
  EXPECT_EQ(driver.readSteps(), 0);

  // Selbst wenn die frühere Ruhephase (bei Position 4) abgelaufen wäre:
  // Die neue Änderung auf Position 8 hat die Entprellung neu gestartet.
  advanceMillis(DebounceConfig::kEncoderMs / 2);
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(DebounceConfig::kEncoderMs / 2);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest, returns_step_exactly_at_debounce_boundary) {
  //{{{
  testEncoder.position = 4;
  driver.readSteps();

  testEncoder.position = 4;
  advanceMillis(DebounceConfig::kEncoderMs - 1);
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(1);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest, accumulates_multiple_full_steps_after_quiet_period) {
  //{{{
  testEncoder.position = 12;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 3);
};
//}}}

TEST_F(EncoderDriverTest, handles_negative_steps_and_remainder) {
  //{{{
  testEncoder.position = -6;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), -1);

  testEncoder.position = -8;
  EXPECT_EQ(driver.readSteps(), 0);
  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), -1);
};
//}}}

TEST_F(EncoderDriverTest, no_output_when_no_new_movement_even_after_many_reads) {
  //{{{
  testEncoder.position = 4;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 1);

  for (int i = 0; i < 20; ++i) {
    advanceMillis(DebounceConfig::kEncoderMs);
    EXPECT_EQ(driver.readSteps(), 0);
  }
};
//}}}

TEST_F(EncoderDriverTest, debounce_resets_on_every_intermediate_movement) {
  //{{{
  testEncoder.position = 4;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(DebounceConfig::kEncoderMs / 3);
  testEncoder.position = 5;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(DebounceConfig::kEncoderMs / 3);
  testEncoder.position = 8;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(DebounceConfig::kEncoderMs / 3);
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 2);

  testEncoder.position = 4;
  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 0);
};
//}}}

TEST_F(EncoderDriverTest, init_captures_initial_delta) {
  //{{{
  testEncoder.position = 100;
  driver.init();

  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 0);
};
//}}}

TEST_F(EncoderDriverTest, large_jump_translates_to_expected_step_count) {
  //{{{
  testEncoder.position = 400;
  driver.readSteps();

  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 100);
};
//}}}

TEST_F(EncoderDriverTest, back_and_forth_movement_cancels_delta) {
  //{{{
  testEncoder.position = 4;
  driver.readSteps();

  testEncoder.position = 0;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(DebounceConfig::kEncoderMs);
  EXPECT_EQ(driver.readSteps(), 0);
};
//}}}
