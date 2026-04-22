CC  = avr-gcc
CXX = avr-g++
TESTCC = g++

MCU   = atmega328p
F_CPU = 16000000UL

CFLAGS   = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DARDUINO=10800 -DARDUINO_AVR_UNO -Os -ffunction-sections -fdata-sections
CXXFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DARDUINO=10800 -DARDUINO_AVR_UNO --std=c++11	-Wcpp -Os -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections
TESTCC_FLAGS = -std=c++20 -Wall  -Wextra  -Wno-unused-variable -Wno-unused-parameter -pthread -g

LIBRARIES = includes/libraries
INCLUDES = \
-I$(LIBRARIES)/ArduinoCore-avr/cores/arduino \
-I$(LIBRARIES)/ArduinoCore-avr/variants/standard \
-I$(LIBRARIES)/ArduinoCore-avr/libraries/EEPROM/src \
-I$(LIBRARIES)/Arduino-Temperature-Control-Library \
-I$(LIBRARIES)/Encoder \
-I$(LIBRARIES)/LiquidCrystal_I2C \
-I$(LIBRARIES)/OneWire \
-I$(LIBRARIES)/ArduinoCore-avr/libraries/Wire/src \
-I$(LIBRARIES)/ArduinoCore-avr/libraries/Wire/src/utility \
-I$(LIBRARIES)/ArduinoCore-avr/libraries/SoftwareSerial/src \
-I$(LIBRARIES)/LiquidCrystal_I2C \
-I$(LIBRARIES) \
-Iincludes \

GTEST_ROOT = $(LIBRARIES)/googletest/googletest
TEST_INCLUDES = \
	-I$(LIBRARIES) \
	-I$(GTEST_ROOT) \
	-I$(GTEST_ROOT)/include \
    -Itests \
	-Iincludes


CORE_SRC_C = (wildcard src/*.c)
CORE_SRC_CXX = (wildcard src/*.cpp)

.PHONY: all tests clean


all:
	@mkdir -p build

	# ===== C Core =====
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring.c -o build/wiring.o
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_digital.c -o build/wiring_digital.o
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_analog.c -o build/wiring_analog.o
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_pulse.c -o build/wiring_pulse.o
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_shift.c -o build/wiring_shift.o
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/libraries/Wire/src/utility/twi.c -o build/twi.o
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/WInterrupts.c -o build/WInterrupts.o

	# ===== C++ Core =====
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/Print.cpp -o build/Print.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/Stream.cpp -o build/Stream.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/WString.cpp -o build/WString.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/HardwareSerial.cpp -o build/HardwareSerial.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp -o build/SoftwareSerial.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/libraries/Wire/src/Wire.cpp -o build/Wire.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/HardwareSerial0.cpp -o build/HardwareSerial0.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/main.cpp -o build/arduino_main.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/LiquidCrystal_I2C/LiquidCrystal_I2C.cpp -o build/LiquidCrystal.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/OneWire/OneWire.cpp -o build/OneWire.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/Arduino-Temperature-Control-Library/DallasTemperature.cpp -o build/DallasTemperature.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/Encoder/Encoder.cpp -o build/Encoder.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/linker_stubs.cpp -o build/linker_stubs.o
	# ===== Heizungssteuerung =====
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/main.cpp -o build/heizungsteuerungmain.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/controller.cpp -o build/controller.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/devicegroups.cpp -o build/devicegroups.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/hardwaredrivers.cpp -o build/hardwaredrivers.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/displaydriver.cpp -o build/displaydriver.o
	

	# ===== Link =====
	@$(CXX) -mmcu=$(MCU) -Wl,--gc-sections build/*.o -o build/main.elf 
	@echo "linked to build/main.elf"
	@avr-objcopy -O ihex build/main.elf build/main.hex
	@echo ".elf and .hex file created"

test: 	
	@mkdir -p build_test
	#============ Compiling ==========================
	@$(TESTCC) $(TESTCC_FLAGS) $(TEST_INCLUDES) -DTEST_BUILD -c tests/input_handle_test.cpp -o build_test/input_switches_test.o
	@$(TESTCC) $(TESTCC_FLAGS) $(TEST_INCLUDES) -c tests/test_devices.cpp -o build_test/test_devices.o
	@$(TESTCC) $(TESTCC_FLAGS) $(TEST_INCLUDES) -c $(GTEST_ROOT)/src/gtest-all.cc -o build_test/gtest-all.o
	@$(TESTCC) $(TESTCC_FLAGS) $(TEST_INCLUDES) -DTEST_BUILD -c src/controller.cpp -o build_test/test_controller.o

	#============= Linking ==============================
	@$(TESTCC) $(TESTCC_FLAGS) build_test/*.o  -o build_test/unit_tests
	@echo "build_tests/unit_tests created"
	

clean:
	@cd build && find . -name "*.o" -delete
	@cd build_test && find . -name "*.o" -delete 
	@echo "object files removed from build directory"
	@rm -f compile_commands.json
	@rm -f build_test/unit_tests
	@echo "compile commands und executable unit_tests removed" 
