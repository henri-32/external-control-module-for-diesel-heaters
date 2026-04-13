CC  = avr-gcc
CXX = avr-g++

MCU   = atmega328p
F_CPU = 16000000UL

CFLAGS   = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DARDUINO=10800 -DARDUINO_AVR_UNO -Os -ffunction-sections -fdata-sections
CXXFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DARDUINO=10800 -DARDUINO_AVR_UNO --std=c++11	-Wcpp -Os -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections

LIBRARIES = includes/libraries

INCLUDES = \
-I$(LIBRARIES)/ArduinoCore-avr/cores/arduino \
-I$(LIBRARIES)/ArduinoCore-avr/variants/standard \
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

CORE_SRC_C = (wildcard src/*.c)
CORE_SRC_CXX = (wildcard src/*.cpp)


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

	# ===== Heizungssteuerung =====
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/main.cpp -o build/heizungsteuerungmain.o
	

	# ===== Link =====
	@$(CXX) -mmcu=$(MCU) -Wl,--gc-sections build/*.o -o build/main.elf 
	@echo "linked to build/main.elf"
	@avr-objcopy -O ihex build/main.elf build/main.hex
	@echo ".elf and .hex file created"

clean:
	@cd build && find . -name "*.o" -delete
	@echo "object files removed from build directory"
	@rm compile_commands.json
	@echo "compile commands removed" 
