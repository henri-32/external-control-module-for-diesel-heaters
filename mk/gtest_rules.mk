include mk/gtest_variables.mk
$(GTEST_OBJ): $(GTEST_ROOT)/src/gtest-all.cc
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -c $< -o $@
 
$(GTEST_LIB): $(GTEST_OBJ)
	@ar rcs $@ $^ 
	@echo "gtest library build" 

$(GTEST_DEBUG_OBJ): $(GTEST_ROOT)/src/gtest-all.cc $(TEST_DEBUG_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TESTCC) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@

$(GTEST_DEBUG_LIB): $(GTEST_DEBUG_OBJ)
	@ar rcs $@ $^
	@echo "gtest debug library build"

