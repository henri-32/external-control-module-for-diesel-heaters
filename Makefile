include mk/globals.mk
include mk/toolchain.mk
include mk/mcu_rules.mk
include mk/gtest_rules.mk
include mk/test_rules.mk

.DEFAULT_GOAL := mcu

.PHONY: mcu setup install compiledb_mcu compiledb_test test test_debug run_test clean ccache_prep

MAKEFLAGS += --no-print-directory


mcu: ccache_prep $(MCU_BUILD_DIR)/main.hex

mcu_logged: 
	@mkdir -p .logs 
	@$(MAKE) mcu > $(BUILD_LOG) 
	@echo 'Build log for "mcu" target written to $(BUILD_LOG)' 

test: ccache_prep $(TEST_BUILD_DIR)/unit_tests 

run_test: test
	python3 scripts/run_test.py unit_tests

run_integrationtest: integrationtest
	python3 scripts/run_test.py integration_test

test_debug: ccache_prep $(TEST_DEBUG_BIN)

test_logged: 
	@mkdir -p .logs 
	@$(MAKE) test > $(BUILD_LOG) 2>&1 
	@echo 'Build Log for "test" target written to $(BUILD_LOG)' 

integrationtest: ccache_prep $(INTEGRATIONTEST_BUILD_DIR)/integration_test

clean:
	@rm -rf $(MCU_BUILD_DIR) $(TEST_BUILD_DIR) $(TEST_DEBUG_BUILD_DIR) $(INTEGRATIONTEST_BUILD_DIR) compile_commands.json
	@echo "build artifacts, compile commands and test binary removed"

setup:
	@scripts/setup.sh

requirements.txt: requirements.in
	python -m piptools compile

install: requirements.txt
	venv/bin/pip install -r requirements.txt

compiledb_mcu:
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

-include $(MCU_DEPS) $(TEST_DEPS) $(TEST_DEBUG_DEPS)
