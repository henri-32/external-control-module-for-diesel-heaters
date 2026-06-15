include mk/globals.mk
include mk/toolchain.mk
include mk/mcu_rules.mk
include mk/gtest_rules.mk
include mk/test_rules.mk

.DEFAULT_GOAL := mcu

.PHONY: targets mcu mcu_logged setup install compiledb_mcu compiledb_fast compiledb_test compiledb_test_fast compiledb_integrationtest compiledb_integrationtest_fast test test_debug test_logged integrationtest integrationtest_debug run_test run_test_debug run_integrationtest run_integrationtest_debug clean ccache_prep

MAKEFLAGS += --no-print-directory

targets: 
	@printf '%s\n' \
		targets \
		mcu \
		mcu_logged \
		test \
		test_debug \
		test_logged \
		run_test \
		run_test_debug \
		integrationtest \
		integrationtest_debug \
		run_integrationtest \
		run_integrationtest_debug \
		compiledb_mcu \
		compiledb_fast \
		compiledb_test \
		compiledb_test_fast \
		compiledb_integrationtest \
		compiledb_integrationtest_fast \
		clean \
		setup \
		install

mcu: ccache_prep $(MCU_BUILD_DIR)/main.hex

mcu_logged: clean
	@mkdir -p .logs 
	@$(MAKE) mcu > $(BUILD_LOG) 
	@echo 'Build log for "mcu" target written to $(BUILD_LOG)' 

test: ccache_prep $(TEST_BUILD_DIR)/unit_tests 

test_debug: ccache_prep $(TEST_DEBUG_BIN)

test_logged: clean 
	@mkdir -p .logs 
	@$(MAKE) test > $(BUILD_LOG) 2>&1 
	@echo 'Build Log for "test" target written to $(BUILD_LOG)' 

run_test: test
	@python3 scripts/run_test.py unit_tests

run_test_debug: test_debug 
	python3 scripts/run_test.py unit_tests_debug

integrationtest: ccache_prep $(INTEGRATIONTEST_BUILD_DIR)/integration_test

integrationtest_debug: ccache_prep $(INTEGRATIONTEST_DEBUG_BIN)

run_integrationtest: integrationtest
	python3 scripts/run_test.py integration_test

run_integrationtest_debug: integrationtest_debug 
	python3 scripts/run_test.py integrationtest_debug

compiledb_mcu:
	@rm -f compile_commands.json
	@$(MAKE) clean 
	@bear -- $(MAKE) USE_CCACHE=0 mcu  
	@echo "compiledb updated to production build"

compiledb_fast: 
	@rm -f compile_commands.json 
	@bear -- $(MAKE) USE_CCACHE=1 mcu 
	@echo "compiledb updated to production build"

compiledb_test:
	@rm -f compile_commands.json
	@$(MAKE) clean
	@bear -- $(MAKE) USE_CCACHE=0 test
	@echo "compiledb updated to test build"

compiledb_test_fast: 
	@rm -f compile_commands.json
	@bear -- $(MAKE) USE_CCACHE=1 test
	@echo "compiledb updated to test build"

compiledb_integrationtest: 
	@rm -f compile_commands.json 
	@$(MAKE) clean 
	@bear -- $(MAKE) integrationtest
	@echo 'compiledb updated to integrationtest build'

compiledb_integrationtest_fast: 
	@rm -f compile_commands.json 
	@$(MAKE) clean 
	@bear -- $(MAKE) USE_CCACHE=1 integrationtest
	@echo 'compiledb updated to integrationtest build' 


clean:
	@rm -rf $(MCU_BUILD_DIR) $(TEST_BUILD_DIR) $(TEST_DEBUG_BUILD_DIR) $(INTEGRATIONTEST_BUILD_DIR) $(INTEGRATIONTEST_DEBUG_BUILD_DIR) compile_commands.json
	@echo "build artifacts, compile commands and test binary removed"

setup: 
	@scripts/setup.sh
	@$(MAKE) install


install: requirements.txt
	venv/bin/pip install -r requirements.txt

requirements.txt: requirements.in
	python -m piptools compile


-include $(MCU_DEPS) $(TEST_DEPS) $(TEST_DEBUG_DEPS) $(INTEGRATIONTEST_DEPS) $(INTEGRATIONTEST_DEBUG_DEPS)
