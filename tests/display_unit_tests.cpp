#include "ArduinoStubs.h"
#include "display_driver.h"
#include "test_devices.h"
#include <gtest/gtest.h>

TEST(InitTests, displayInit) {
  //{{{
  //Pretty senseless init() testing but now it's written
  OutputDevicesIntent outputIntent;
  TestDisplayHardware m_display;
  DisplayDriver display{m_display, outputIntent.displayContent,
                        outputIntent.lcd_state};
  display.init();

  EXPECT_EQ(m_display.m_init_called, 1);
  EXPECT_EQ(m_display.no_backlight_calls, 1); 
  EXPECT_EQ(m_display.no_display_calls, 1); 
  EXPECT_EQ(m_display.clear_calls, 1); 
};
//}}}
class DisplayTest : public ::testing::Test {
protected:
  OutputDevicesIntent outputIntent;
  TestDisplayHardware display;

  DisplayDriver driver{display, outputIntent.displayContent,
                       outputIntent.lcd_state};

  void SetUp() override { ArduinoStubSpies::setMillis(100); }
};

TEST_F(DisplayTest, init_gets_called) {
  //{{{
  driver.init();

  ASSERT_EQ(display.m_init_called, true);
  EXPECT_EQ(display.no_backlight_calls, 1);
  EXPECT_EQ(display.no_display_calls, 1);
  EXPECT_EQ(display.clear_calls, 1);
}
//}}}

TEST_F(DisplayTest, update_off_turns_display_off_without_writing) {
  //{{{
  outputIntent.lcd_state = OutputDevicesIntent::LcdStateIntent::Off;

  driver.update();

  EXPECT_EQ(display.no_backlight_calls, 1);
  EXPECT_EQ(display.no_display_calls, 1);
  EXPECT_TRUE(display.printed_lines.empty());
}
//}}}

TEST_F(DisplayTest, update_page1_writes_expected_lines) {
  //{{{
  outputIntent.lcd_state = OutputDevicesIntent::LcdStateIntent::Page1;
  outputIntent.displayContent.temp_c = 21.3F;
  outputIntent.displayContent.status.target_tempC = 19.8F;
  outputIntent.displayContent.status.state = HeaterStatus::State::On;
  outputIntent.displayContent.status.mode = HeaterStatus::Mode::Power;

  driver.update();

  EXPECT_EQ(display.backlight_calls, 1);
  EXPECT_EQ(display.display_calls, 1);
  ASSERT_GE(display.printed_lines.size(), 8U);
  EXPECT_EQ(display.printed_lines[1], "Temp.:     21.3 C");
  EXPECT_EQ(display.printed_lines[3], "Solltemp.: 19.8 C");
  EXPECT_EQ(display.printed_lines[5], "Zustand:   ON");
  EXPECT_EQ(display.printed_lines[7], "Mode:      POWER");
}
//}}}

TEST_F(DisplayTest, update_with_same_content_does_not_rewrite_lines) {
  //{{{
  outputIntent.lcd_state = OutputDevicesIntent::LcdStateIntent::Page1;
  outputIntent.displayContent.temp_c = 20.0F;
  outputIntent.displayContent.status.target_tempC = 18.5F;
  outputIntent.displayContent.status.state = HeaterStatus::State::Off;
  outputIntent.displayContent.status.mode = HeaterStatus::Mode::Temp;

  driver.update();
  const size_t printed_after_first_update = display.printed_lines.size();

  driver.update();

  EXPECT_EQ(display.printed_lines.size(), printed_after_first_update);
}
//}}}
