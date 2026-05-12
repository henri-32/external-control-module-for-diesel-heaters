#include "ArduinoStubs.h"
#include "encoder_driver.h"
#include "test_devices.h"
#include <gtest/gtest.h>
using namespace ArduinoStubSpies;

// Test-Hinweise für EncoderDriver:
// - position simuliert den absoluten Hardware-Encoder-Zählerstand.
// - Der Treiber arbeitet auf Deltas (current - prev) und zählt nur ganze
//   Schritte: 4 Impulse == 1 Schritt (steps = m_delta / 4).
// - Jeder TEST_F startet mit frischem Fixture (m_prev=0, m_delta=0,
//   m_last_change_ms=0). Es gibt keinen Zustandstransfer zwischen Tests.
// - Nach jeder Positionsänderung wird m_last_change_ms neu gesetzt.
//   Solange debounceConfig::encoder seit der letzten Änderung nicht
//   abgelaufen ist, liefert readSteps() immer 0.
// - Änderungen werden nicht verworfen: Sie werden in m_delta gesammelt.
//   Ausgegeben werden nur ganze Schritte, der Rest bleibt im Delta.

class EncoderDriverTest : public ::testing::Test {
protected:
  InputDevicesDataSet inputDataBuffer;
  TestEncoderHardware testEncoder;
  EncoderDriver driver{testEncoder};

  void SetUp() override {
    setMillis(0);
    testEncoder.position = 0;
  };
};

TEST_F(EncoderDriverTest, returnsZero_whenHardwarePositionIsZero) {
  //{{{
  testEncoder.position = 0;
  EXPECT_EQ(driver.readSteps(), 0);
};
//}}}

TEST_F(EncoderDriverTest, returnsZero_withinDebounceInterval) {
  //{{{
  // Innerhalb Debounce
  testEncoder.position = 4;
  EXPECT_EQ(driver.readSteps(), 0);

  // Immer noch innerhalb Debounce
  advanceMillis(debounceConfig::encoder - 1);
  testEncoder.position = 4;
  EXPECT_EQ(driver.readSteps(), 0);
}
//}}}

TEST_F(EncoderDriverTest, returnsOneStep_afterDebounceWithStablePosition) {
  //{{{
  // Erstes Polling erkennt 0 -> 4, speichert Delta und startet Debounce.
  // Daher hier noch keine Schritt-Ausgabe.
  testEncoder.position = 4;
  driver.readSteps();

  // Nach Ablauf der Debounce-Zeit:
  // diff == 0, m_delta bleibt 4, Ausgabe 4/4 == 1.
  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest, carriesRemainderStepsIntoLaterReads) {
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

TEST_F(EncoderDriverTest, changeResetsDebounce_evenWhenPreviousDebounceWouldHaveElapsed) {
  //{{{
  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 0);

  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 1);

  // Debounce seit letzter Positionsänderung ist noch nicht vorbei.
  testEncoder.position = 8; 
  EXPECT_EQ(driver.readSteps(), 0); 

  advanceMillis(debounceConfig::encoder / 2);
  EXPECT_EQ(driver.readSteps(), 0); 

  // Die Änderung auf Position 8 setzt den Debounce-Timer neu.
  advanceMillis(debounceConfig::encoder / 2); 
  EXPECT_EQ(driver.readSteps(), 1); 
  
};
//}}}


TEST_F(EncoderDriverTest, repeatedReadsStillWaitUntilDebounceAfterChange) {
  //{{{
  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 0);

  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 1);

  // Debounce seit letzter Positionsänderung ist noch nicht vorbei.
  testEncoder.position = 8; 
  advanceMillis(debounceConfig::encoder / 2);
  EXPECT_EQ(driver.readSteps(), 0); 

  // Auch wenn die frühere Ruhephase (bei Position 4) abgelaufen wäre:
  // Die neue Änderung auf Position 8 hat den Debounce neu gestartet.
  advanceMillis(debounceConfig::encoder / 2); 
  EXPECT_EQ(driver.readSteps(), 0); 

  advanceMillis(debounceConfig::encoder / 2); 
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest, returnsStepExactlyAtDebounceBoundary) {
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

TEST_F(EncoderDriverTest, accumulatesMultipleFullStepsAfterQuietPeriod) {
  //{{{
  testEncoder.position = 12;
  driver.readSteps();

  testEncoder.position = 12;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 3);
};
//}}}

TEST_F(EncoderDriverTest, handlesNegativeStepsAndRemainder) {
  //{{{
  testEncoder.position = -6;
  driver.readSteps();

  testEncoder.position = -6;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), -1);

  testEncoder.position = -8;
  driver.readSteps();
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), -1);
};
//}}}

TEST_F(EncoderDriverTest, noOutputWhenNoNewMovementEvenAfterManyReads) {
  //{{{
  testEncoder.position = 4;
  driver.readSteps();

  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 1);

  for (int i = 0; i < 5; ++i) {
    advanceMillis(debounceConfig::encoder);
    EXPECT_EQ(driver.readSteps(), 0);
  }
};
//}}}

TEST_F(EncoderDriverTest, debounceResetsOnEveryIntermediateMovement) {
  //{{{
  testEncoder.position = 1;
  driver.readSteps();

  advanceMillis(debounceConfig::encoder / 3);
  testEncoder.position = 2;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder / 3);
  testEncoder.position = 3;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder / 3);
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 0);

  testEncoder.position = 4;
  driver.readSteps();
  testEncoder.position = 4;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 1);
};
//}}}

TEST_F(EncoderDriverTest, initCapturesInitialDeltaAndReturnsItAfterDebounce) {
  //{{{
  testEncoder.position = 100;
  driver.init();

  testEncoder.position = 100;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 0);
};
//}}}

TEST_F(EncoderDriverTest, largeJumpTranslatesToExpectedStepCount) {
  //{{{
  testEncoder.position = 400;
  driver.readSteps();

  testEncoder.position = 400;
  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 100);
};
//}}}

TEST_F(EncoderDriverTest, backAndForthMovementCancelsDelta) {
  //{{{
  testEncoder.position = 2;
  driver.readSteps();

  testEncoder.position = 0;
  EXPECT_EQ(driver.readSteps(), 0);

  advanceMillis(debounceConfig::encoder);
  EXPECT_EQ(driver.readSteps(), 0);
};
//}}}
