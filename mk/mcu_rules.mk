include mk/globals.mk
include mk/mcu_variables.mk

$(MCU_BUILD_DIR)/main.hex: $(MCU_BUILD_DIR)/main.elf
	@avr-objcopy -O ihex $< $@
	@echo ".hex file created\n========================="

$(MCU_BUILD_DIR)/main.elf: $(MCU_OBJS)
	@$(CXX) -mmcu=$(MCU) -Wl,--gc-sections $^ -o $@
	@printf "LINK $^\n===> $@\n"

$(MCU_BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(MCU_CPPFLAGS) $(CFLAGS) $(INCLUDES) -c $< -o $@
	@echo 'CC $< -> $@'

$(MCU_BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(MCU_CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
	@echo 'CXX $< -> $@'

