include mk/globals.mk


TEST_INCLUDES := \
	-I$(LIBRARIES) \
	-I$(GTEST_ROOT) \
	-I$(GTEST_ROOT)/include \
	-Itests \
	-Iincludes

TEST_CPP_SRCS := \
	src/controller.cpp \
	src/devicegroups.cpp \
	src/temperature_sensor_driver.cpp \
	src/display_driver.cpp \
	src/relais.cpp \
	src/pushbuttons.cpp \
	src/toggle_switches.cpp \
	src/encoder_driver.cpp \
	tests/test_devices.cpp \
	tests/ArduinoStubs.cpp \
	tests/systemController_white_box_tests.cpp \
	tests/systemController_black_box_tests.cpp \
	tests/display_unit_tests.cpp \
	tests/relais_unit_tests.cpp \
	tests/encoder_driver_unit_test.cpp \
	tests/temperature_sensor_unit_test.cpp 

INTEGRATIONTEST_CPP_SRCS:= \
	src/controller.cpp \
	src/devicegroups.cpp \
	src/temperature_sensor_driver.cpp \
	src/display_driver.cpp \
	src/relais.cpp \
	src/pushbuttons.cpp \
	src/toggle_switches.cpp \
	src/encoder_driver.cpp \
	tests/ArduinoStubs.cpp \
	tests/test_devices.cpp \
	tests/systemController_integration_test.cpp

INTEGRATIONTEST_OBJS := $(addprefix $(INTEGRATIONTEST_BUILD_DIR)/,$(INTEGRATIONTEST_CPP_SRC:.cpp=.o))
INTEGRATIONTEST_DEPS := $(INTEGRATIONTEST_OBJS:.o=.d)


TEST_CPPFLAGS := -DTEST_BUILD
TEST_CXXFLAGS := -std=c++20 -Wall -Wextra -pthread 
TEST_DEBUGFLAGS := -g -MMD -MP -fsanitize=address,undefined

TEST_PCH := includes/pch_test.h
TEST_PCH_GCH := $(TEST_BUILD_DIR)/pch_test.h.gch
TEST_PCH_FLAGS := -include pch_test.h -Winvalid-pch 

TEST_DEBUG_BUILD_DIR := build_test_debug
TEST_DEBUG_PCH_GCH := $(TEST_DEBUG_BUILD_DIR)/pch_test.h.gch
TEST_DEBUG_CPP_OBJS = $(addprefix $(TEST_DEBUG_BUILD_DIR)/,$(TEST_CPP_SRCS:.cpp=.o))
TEST_DEBUG_OBJS = $(TEST_DEBUG_CPP_OBJS)
TEST_DEBUG_DEPS = $(TEST_DEBUG_OBJS:.o=.d)
TEST_DEBUG_BIN := $(TEST_DEBUG_BUILD_DIR)/unit_tests
	
TEST_OBJS := $(addprefix $(TEST_BUILD_DIR)/,$(TEST_CPP_SRCS:.cpp=.o))
TEST_DEPS := $(TEST_OBJS:.o=.d)
