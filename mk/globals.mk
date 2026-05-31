ifndef GLOBALS_INCLUDED 
GLOBALS_INCLUDED := 1

.DEFAULT_GOAL := mcu
MCU_BUILD_DIR := build
TEST_BUILD_DIR := build_test
TEST_DEBUG_BUILD_DIR := build_test_debug
INTEGRATIONTEST_BUILD_DIR := build_integrationtest

BUILD_LOG := .logs/build.log
LIBRARIES := includes/libraries
GTEST_LIB :=$(TEST_BUILD_DIR)/libtest.a
GTEST_ROOT := $(LIBRARIES)/googletest/googletest


ccache_prep:
ifeq ($(USE_CCACHE),1)
	@mkdir -p "$(CCACHE_TEMPDIR)"
endif


endif
