.DEFAULT_GOAL := all

.PHONY: all setup install compiledb compiledb_test test test_debug run_test clean

MAKEFLAGS += --no-print-directory

setup:
	@scripts/setup.sh

requirements.txt: requirements.in
	python -m piptools compile

install: requirements.txt
	venv/bin/pip install -r requirements.txt

compiledb:
	@rm -f compile_commands.json
	@make clean 
	@bear -- $(MAKE) all 
	@echo "compiledb updated to production build"

compiledb_test:
	@rm -f compile_commands.json
	@make clean
	@bear -- $(MAKE) test
	@echo "compiledb updated to test build"

CC :=  avr-gcc
CXX :=  avr-g++
TESTCC := ccache g++

MCU := atmega328p
F_CPU := 16000000UL

LIBRARIES := includes/libraries
BUILD_DIR := build
TEST_BUILD_DIR := build_test

COMMON_DEFINES := -DF_CPU=$(F_CPU) -DARDUINO=10800 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR -DDECIMAL_DIG=__DECIMAL_DIG__
COMMON_OPT := -Os -ffunction-sections -fdata-sections

CPPFLAGS := $(COMMON_DEFINES)
INCLUDES := \
	-I$(LIBRARIES)/ArduinoCore-avr/cores/arduino \
	-I$(LIBRARIES)/ArduinoCore-avr/variants/standard \
	-I$(LIBRARIES)/ArduinoCore-avr/libraries/EEPROM/src \
	-I$(LIBRARIES)/Arduino-Temperature-Control-Library \
	-I$(LIBRARIES)/Encoder \
	-I$(LIBRARIES)/LiquidCrystal_I2C \
	-I$(LIBRARIES)/OneWire \
	-I$(LIBRARIES)/ArduinoCore-avr/libraries/Wire/src \
	-I$(LIBRARIES)/ArduinoCore-avr/libraries/Wire/src/utility \
	-I$(LIBRARIES)/ArduinoCore-avr/libraries/SoftwareSerial/src \
	-I$(LIBRARIES) \
	-Iincludes

CFLAGS := -mmcu=$(MCU) -std=gnu11 $(COMMON_OPT) -MMD -MP
CXXFLAGS := -mmcu=$(MCU) -std=gnu++11 -Wcpp $(COMMON_OPT) -fno-exceptions -fno-rtti -MMD -MP

GTEST_ROOT := $(LIBRARIES)/googletest/googletest
TEST_INCLUDES := \
	-I$(LIBRARIES) \
	-I$(GTEST_ROOT) \
	-I$(GTEST_ROOT)/include \
	-Itests \
	-Iincludes

TEST_PCH := includes/pch_test.h
TEST_PCH_GCH := $(TEST_BUILD_DIR)/pch_test.h.gch
TEST_PCH_FLAGS := -include pch_test.h -Winvalid-pch 

TEST_CPPFLAGS := -DTEST_BUILD
TEST_CXXFLAGS := -std=c++20 -Wall -Wextra -pthread 
TEST_DEBUGFLAGS := -g -MMD -MP -fsanitize=address,undefined

TEST_DEBUG_BUILD_DIR := build_test_debug
TEST_DEBUG_PCH_GCH := $(TEST_DEBUG_BUILD_DIR)/pch_test.h.gch
TEST_DEBUG_CPP_OBJS = $(addprefix $(TEST_DEBUG_BUILD_DIR)/,$(TEST_CPP_SRCS:.cpp=.o))
TEST_DEBUG_OBJS = $(TEST_DEBUG_CPP_OBJS)
TEST_DEBUG_DEPS = $(TEST_DEBUG_OBJS:.o=.d)
GTEST_DEBUG_OBJ := $(TEST_DEBUG_BUILD_DIR)/gtest-all.o
GTEST_DEBUG_LIB := $(TEST_DEBUG_BUILD_DIR)/libtest.a
TEST_DEBUG_BIN := $(TEST_DEBUG_BUILD_DIR)/unit_tests



AVR_C_SRCS := \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring.c \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_digital.c \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_analog.c \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_pulse.c \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_shift.c \
	$(LIBRARIES)/ArduinoCore-avr/libraries/Wire/src/utility/twi.c \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/WInterrupts.c

AVR_CPP_SRCS := \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/Print.cpp \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/Stream.cpp \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/WString.cpp \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/HardwareSerial.cpp \
	$(LIBRARIES)/ArduinoCore-avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp \
	$(LIBRARIES)/ArduinoCore-avr/libraries/Wire/src/Wire.cpp \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/HardwareSerial0.cpp \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/main.cpp \
	$(LIBRARIES)/LiquidCrystal_I2C/LiquidCrystal_I2C.cpp \
	$(LIBRARIES)/OneWire/OneWire.cpp \
	$(LIBRARIES)/Arduino-Temperature-Control-Library/DallasTemperature.cpp \
	$(LIBRARIES)/Encoder/Encoder.cpp

APP_CPP_SRCS := \
	src/linker_stubs.cpp \
	src/main.cpp \
	src/controller.cpp \
	src/controller_construction.cpp \
	src/devicegroups.cpp \
	src/display_driver.cpp \
	src/relais.cpp \
	src/pushbuttons.cpp \
	src/toggle_switches.cpp \
	src/encoder_driver.cpp \
	src/temperature_sensor_driver.cpp \
	src/library_adapter.cpp

C_OBJS := $(addprefix $(BUILD_DIR)/,$(AVR_C_SRCS:.c=.o))
CPP_OBJS := $(addprefix $(BUILD_DIR)/,$(AVR_CPP_SRCS:.cpp=.o) $(APP_CPP_SRCS:.cpp=.o))
OBJS := $(C_OBJS) $(CPP_OBJS)
DEPS := $(OBJS:.o=.d)

TEST_CPP_SRCS := \
	tests/systemController_white_box_tests.cpp \
	tests/systemController_black_box_tests.cpp \
	tests/display_unit_tests.cpp \
	tests/relais_unit_tests.cpp \
	tests/encoder_driver_unit_test.cpp \
	tests/temperature_sensor_unit_test.cpp \
	tests/test_devices.cpp \
	tests/ArduinoStubs.cpp \
	src/controller.cpp \
	src/devicegroups.cpp \
	src/temperature_sensor_driver.cpp \
	src/display_driver.cpp \
	src/relais.cpp \
	src/pushbuttons.cpp \
	src/toggle_switches.cpp \
	src/encoder_driver.cpp


GTEST_LIB :=$(TEST_BUILD_DIR)/libtest.a
GTEST_OBJ :=$(TEST_BUILD_DIR)/gtest-all.o

TEST_CPP_OBJS := $(addprefix $(TEST_BUILD_DIR)/,$(TEST_CPP_SRCS:.cpp=.o))
TEST_OBJS := $(TEST_CPP_OBJS) $(TEST_CC_OBJS)
TEST_DEPS := $(TEST_OBJS:.o=.d)

all: $(BUILD_DIR)/main.hex

test: $(TEST_BUILD_DIR)/unit_tests

test_debug: $(TEST_DEBUG_BIN)


	 

$(BUILD_DIR)/main.elf: $(OBJS)
	@$(CXX) -mmcu=$(MCU) -Wl,--gc-sections $^ -o $@
	@echo "linked to $@"

$(BUILD_DIR)/main.hex: $(BUILD_DIR)/main.elf
	@avr-objcopy -O ihex $< $@
	@echo ".hex file created"

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


$(TEST_BUILD_DIR)/unit_tests: $(TEST_OBJS) $(GTEST_LIB)
	@$(TESTCC) $(TEST_CXXFLAGS) $^ -o $@
	@echo "build_tests/unit_tests created"

$(GTEST_OBJ): $(GTEST_ROOT)/src/gtest-all.cc
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -c $< -o $@
 
$(GTEST_LIB): $(GTEST_OBJ)
	@ar rcs $@ $^ 
	@echo "gtest library build" 

$(TEST_PCH_GCH): $(TEST_PCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -x c++-header $< -o $@

$(TEST_BUILD_DIR)/%.o: %.cpp $(TEST_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@ 

$(TEST_BUILD_DIR)/%.o: %.cc $(TEST_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@

$(TEST_DEBUG_BIN): $(TEST_DEBUG_OBJS) $(GTEST_DEBUG_LIB)
	@$(TESTCC) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $^ -o $@
	@echo "build_test_debug/unit_tests created"

$(GTEST_DEBUG_OBJ): $(GTEST_ROOT)/src/gtest-all.cc $(TEST_DEBUG_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@

$(GTEST_DEBUG_LIB): $(GTEST_DEBUG_OBJ)
	@ar rcs $@ $^
	@echo "gtest debug library build"

$(TEST_DEBUG_PCH_GCH): $(TEST_PCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_INCLUDES) -x c++-header $< -o $@

$(TEST_DEBUG_BUILD_DIR)/%.o: %.cpp $(TEST_DEBUG_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@

$(TEST_DEBUG_BUILD_DIR)/%.o: %.cc $(TEST_DEBUG_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@

run_test:
	python3 scripts/run_test.py

clean:
	@rm -rf $(BUILD_DIR) $(TEST_BUILD_DIR) $(TEST_DEBUG_BUILD_DIR) compile_commands.json
	@echo "build artifacts, compile commands and test binary removed"

-include $(DEPS) $(TEST_DEPS) $(TEST_DEBUG_DEPS)
