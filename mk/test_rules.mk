include mk/globals.mk
include mk/test_variables.mk

$(TEST_BUILD_DIR)/unit_tests: $(TEST_OBJS) $(GTEST_LIB) $(GTEST_MAIN_LIB)
	@$(TESTCC) $(TEST_CXXFLAGS) $^ -o $@
	@echo "build_tests/unit_tests created"

$(INTEGRATIONTEST_BUILD_DIR)/integration_test: $(INTEGRATIONTEST_OBJS) $(GTEST_LIB) $(GTEST_MAIN_LIB)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CXXFLAGS) $^ -o $@
	@echo "build_integrationtest created"

$(INTEGRATIONTEST_DEBUG_BIN): $(INTEGRATIONTEST_DEBUG_OBJS) $(GTEST_DEBUG_LIB) $(GTEST_MAIN_DEBUG_LIB)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $^ -o $@
	@echo "build_integrationtest_debug/integration_test created"

$(TEST_BUILD_DIR)/%.o: %.cpp $(TEST_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@ 

$(TEST_BUILD_DIR)/%.o: %.cc $(TEST_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@

$(INTEGRATIONTEST_BUILD_DIR)/%.o: %.cpp $(TEST_PCH_GCH) 
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@ 

$(INTEGRATIONTEST_DEBUG_BUILD_DIR)/%.o: %.cpp $(TEST_DEBUG_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@

$(TEST_DEBUG_BIN): $(TEST_DEBUG_OBJS) $(GTEST_DEBUG_LIB) $(GTEST_MAIN_DEBUG_LIB)
	@$(TESTCC) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $^ -o $@
	@echo "build_test_debug/unit_tests created"

$(TEST_DEBUG_BUILD_DIR)/%.o: %.cpp $(TEST_DEBUG_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@

$(TEST_PCH_GCH): $(TEST_PCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -x c++-header $< -o $@

$(TEST_DEBUG_PCH_GCH): $(TEST_PCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_INCLUDES) -x c++-header $< -o $@
