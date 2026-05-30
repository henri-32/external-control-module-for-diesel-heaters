include mk/globals.mk
include mk/toolchain.mk
include mk/mcu_rules.mk
include mk/test_rules.mk

.DEFAULT_GOAL := mcu

.PHONY: mcu setup install compiledb compiledb_test test test_debug run_test clean ccache_prep

MAKEFLAGS += --no-print-directory

setup:
	@scripts/setup.sh

requirements.txt: requirements.in
	python -m piptools compile

install: requirements.txt
	venv/bin/pip install -r requirements.txt

compiledb:
	@rm -f compile_commands.json
	@$(MAKE) clean 
	@bear -- $(MAKE) USE_CCACHE=0 mcu  
	@echo "compiledb updated to production build\n"

compiledb_fast: 
	@rm -f compile_commands.json 
	@bear -- $(MAKE) USE_CCACHE=1 mcu 
	@echo "compiledb updated to production build\n"


compiledb_test:
	@rm -f compile_commands.json
	@$(MAKE) clean
	@bear -- $(MAKE) USE_CCACHE=0 test
	@echo "compiledb updated to test build\n"

compiledb_test_fast: 
	@rm -f compile_commands.json
	@bear -- $(MAKE) USE_CCACHE=1 test
	@echo "compiledb updated to test build\n"


GTEST_OBJ :=$(TEST_BUILD_DIR)/gtest-all.o
GTEST_DEBUG_OBJ := $(TEST_DEBUG_BUILD_DIR)/gtest-all.o
GTEST_DEBUG_LIB := $(TEST_DEBUG_BUILD_DIR)/libtest.a


ccache_prep:
ifeq ($(USE_CCACHE),1)
	@mkdir -p "$(CCACHE_TEMPDIR)"
endif

$(GTEST_OBJ): $(GTEST_ROOT)/src/gtest-all.cc
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -c $< -o $@
 
$(GTEST_LIB): $(GTEST_OBJ)
	@ar rcs $@ $^ 
	@echo "gtest library build" 

$(TEST_PCH_GCH): $(TEST_PCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -x c++-header $< -o $@

$(GTEST_DEBUG_OBJ): $(GTEST_ROOT)/src/gtest-all.cc $(TEST_DEBUG_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@

$(GTEST_DEBUG_LIB): $(GTEST_DEBUG_OBJ)
	@ar rcs $@ $^
	@echo "gtest debug library build"

clean:
	@rm -rf $(MCU_BUILD_DIR) $(TEST_BUILD_DIR) $(TEST_DEBUG_BUILD_DIR) $(INTEGRATIONTEST_BUILD_DIR) compile_commands.json
	@echo "build artifacts, compile commands and test binary removed"

-include $(MCU_DEPS) $(TEST_DEPS) $(TEST_DEBUG_DEPS)
