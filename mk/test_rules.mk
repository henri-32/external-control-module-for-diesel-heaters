include mk/globals.mk
include mk/test_variables.mk

$(TEST_BUILD_DIR)/unit_tests: $(TEST_OBJS) $(GTEST_LIB) $(GTEST_MAIN_LIB)
	@$(TEST_CXX) $(TEST_CXXFLAGS) $^ -o $@
	@printf 'LINK \n$^ \n===> $@\n'
	@printf "\n=========================\n"

$(INTEGRATIONTEST_BUILD_DIR)/integration_test: $(INTEGRATIONTEST_OBJS) $(GTEST_LIB) $(GTEST_MAIN_LIB)
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CXXFLAGS) $^ -o $@
	@echo 'CXX $^ -> $@'
	@printf "\n=========================\n"

$(INTEGRATIONTEST_DEBUG_BIN): $(INTEGRATIONTEST_DEBUG_OBJS) $(GTEST_DEBUG_LIB) $(GTEST_MAIN_DEBUG_LIB)
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $^ -o $@
	@echo 'CXX $^ -> $@'
	@printf "\n=========================\n"

$(TEST_BUILD_DIR)/%.o: %.cpp $(TEST_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@ 
	@echo 'CXX $< -> $@'

$(TEST_BUILD_DIR)/%.o: %.cc $(TEST_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@
	@echo 'CXX $^ -> $@'

$(INTEGRATIONTEST_BUILD_DIR)/%.o: %.cpp $(TEST_PCH_GCH) 
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@ 
	@echo 'CXX $^ -> $@'

$(INTEGRATIONTEST_DEBUG_BUILD_DIR)/%.o: %.cpp $(TEST_DEBUG_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@
	@echo 'CXX $^ -> $@'

$(TEST_DEBUG_BIN): $(TEST_DEBUG_OBJS) $(GTEST_DEBUG_LIB) $(GTEST_MAIN_DEBUG_LIB)
	@$(TEST_CXX) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $^ -o $@
	@echo 'LINK \n$^\n===> $@\n'
	@echo "build_test_debug/unit_tests created"

$(TEST_DEBUG_BUILD_DIR)/%.o: %.cpp $(TEST_DEBUG_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@
	@echo 'CXX $^ -> $@'

$(TEST_PCH_GCH): $(TEST_PCH)
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -x c++-header $< -o $@
	@echo 'PCH compiled' 

$(TEST_DEBUG_PCH_GCH): $(TEST_PCH)
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_INCLUDES) -x c++-header $< -o $@
	@echo 'PCH compiled' 
