include mk/gtest_variables.mk
$(GTEST_OBJ): $(GTEST_ROOT)/src/gtest-all.cc
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -c $< -o $@
	@echo 'CC $< -> $@'
 
$(GTEST_LIB): $(GTEST_OBJ)
	@ar rcs $@ $^ 
	@echo "LINK $^ -> $@" 

$(GTEST_MAIN_OBJ): $(GTEST_ROOT)/src/gtest_main.cc
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -c $< -o $@
	@echo 'CC $< -> $@'

$(GTEST_MAIN_LIB): $(GTEST_MAIN_OBJ)
	@ar rcs $@ $^
	@echo 'LINK $^ \n==> $@\n' 
	@echo "gtest_main library build"

$(GTEST_DEBUG_OBJ): $(GTEST_ROOT)/src/gtest-all.cc $(TEST_DEBUG_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@
	@echo 'CC $< -> $@'

$(GTEST_DEBUG_LIB): $(GTEST_DEBUG_OBJ)
	@ar rcs $@ $^
	@echo 'LINK $^ \n==> $@\n' 
	@echo "gtest debug library build"

$(GTEST_MAIN_DEBUG_OBJ): $(GTEST_ROOT)/src/gtest_main.cc $(TEST_DEBUG_PCH_GCH)
	@mkdir -p $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_DEBUGFLAGS) $(TEST_PCH_FLAGS) $(TEST_INCLUDES) -c $< -o $@
	@echo 'CC $< -> $@'

$(GTEST_MAIN_DEBUG_LIB): $(GTEST_MAIN_DEBUG_OBJ)
	@ar rcs $@ $^
	@echo 'LINK $^ \n==> $@\n' 
	@echo "gtest_main debug library build"

