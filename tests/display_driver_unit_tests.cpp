#include "displaydriver.h"
#include "test_devices.h"
#include <gtest/gtest.h>

class DisplayDriverTest : public ::testing::Test {
protected:
  ControllerOutputIntent COI;
  TestDisplay display;

  DisplayDriver driver{display, COI.displayContent, COI.lcd_stateIntent};
};

//Sinnloser Test, nur für den Anfang 
TEST_F(DisplayDriverTest, init_gets_called) {
  driver.init();

  ASSERT_EQ(display.m_init_called, true);
}
