#include "displaydriver.h"
#include "test_devices.h"
#include <gtest/gtest.h>

class DisplayDriverTest : public ::testing::Test {
protected:
  ControllerOutputIntent COI;
  TestDisplay display;

  DisplayDriver driver{display, COI.displayContent, COI.lcd_stateIntent};
};

TEST_F(DisplayDriverTest, init_gets_called) {
  driver.init();

  ASSERT_EQ(display.m_init_called, true);
  EXPECT_EQ(display.no_backlight_calls, 1);
  EXPECT_EQ(display.no_display_calls, 1);
  EXPECT_EQ(display.clear_calls, 1);
}

TEST_F(DisplayDriverTest, update_off_turns_display_off_without_writing) {
  COI.lcd_stateIntent = ControllerOutputIntent::LCD_StateIntent::OFF;

  driver.update();

  EXPECT_EQ(display.no_backlight_calls, 1);
  EXPECT_EQ(display.no_display_calls, 1);
  EXPECT_TRUE(display.printed_lines.empty());
}

TEST_F(DisplayDriverTest, update_page1_writes_expected_lines) {
  COI.lcd_stateIntent = ControllerOutputIntent::LCD_StateIntent::Page1;
  COI.displayContent.temp_c = 21.3F;
  COI.displayContent.target_tempC = 19.8F;
  COI.displayContent.heatingState = HeaterStatus::HeatingState::ON;
  COI.displayContent.mode = HeaterStatus::Mode::POWER;

  driver.update();

  EXPECT_EQ(display.backlight_calls, 1);
  EXPECT_EQ(display.display_calls, 1);
  ASSERT_GE(display.printed_lines.size(), 8U);
  EXPECT_EQ(display.printed_lines[1], "Temp.:     21.3 C");
  EXPECT_EQ(display.printed_lines[3], "Solltemp.: 19.8 C");
  EXPECT_EQ(display.printed_lines[5], "Zustand:   ON");
  EXPECT_EQ(display.printed_lines[7], "Mode:      POWER");
}

TEST_F(DisplayDriverTest, update_with_same_content_does_not_rewrite_lines) {
  COI.lcd_stateIntent = ControllerOutputIntent::LCD_StateIntent::Page1;
  COI.displayContent.temp_c = 20.0F;
  COI.displayContent.target_tempC = 18.5F;
  COI.displayContent.heatingState = HeaterStatus::HeatingState::OFF;
  COI.displayContent.mode = HeaterStatus::Mode::TEMP;

  driver.update();
  const size_t printed_after_first_update = display.printed_lines.size();

  driver.update();

  EXPECT_EQ(display.printed_lines.size(), printed_after_first_update);
}
