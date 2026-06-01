include mk/globals.mk

GTEST_OBJ :=$(TEST_BUILD_DIR)/gtest-all.o
GTEST_MAIN_OBJ := $(TEST_BUILD_DIR)/gtest_main.o
GTEST_DEBUG_OBJ := $(TEST_DEBUG_BUILD_DIR)/gtest-all.o
GTEST_MAIN_DEBUG_OBJ := $(TEST_DEBUG_BUILD_DIR)/gtest_main.o
GTEST_DEBUG_LIB := $(TEST_DEBUG_BUILD_DIR)/libtest.a
GTEST_MAIN_LIB := $(TEST_BUILD_DIR)/libgtest_main.a
GTEST_MAIN_DEBUG_LIB := $(TEST_DEBUG_BUILD_DIR)/libgtest_main.a

