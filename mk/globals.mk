ifndef GLOBALS_INCLUDED 
GLOBALS_INCLUDED := 1

.DEFAULT_GOAL := mcu
TEST_BUILD_DIR := build_test
BUILD_LOG := .logs/build.log
LIBRARIES := includes/libraries
GTEST_LIB :=$(TEST_BUILD_DIR)/libtest.a
GTEST_ROOT := $(LIBRARIES)/googletest/googletest
INTEGRATIONTEST_BUILD_DIR := build_integrationtest
MCU_BUILD_DIR := build

endif
