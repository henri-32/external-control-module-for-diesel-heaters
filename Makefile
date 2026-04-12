CC  = avr-gcc
CXX = avr-g++

MCU   = atmega328p
F_CPU = 16000000UL

CFLAGS   = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DARDUINO=10800 -DARDUINO_AVR_UNO -Os
CXXFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DARDUINO=10800 -DARDUINO_AVR_UNO --std=c++11	-Wcpp -Os

LIBRARIES = includes/libraries

INCLUDES = \
-I$(LIBRARIES)/ArduinoCore-avr/cores/arduino \
-I$(LIBRARIES)/ArduinoCore-avr/cores/arduino/avr \
-I$(LIBRARIES)/ArduinoCore-avr/variants/standard \
-I$(LIBRARIES)/Arduino-Temperature-Control-Library \
-I$(LIBRARIES)/Encoder \
-I$(LIBRARIES)/LiquidCrystal/src \
-I$(LIBRARIES)/OneWire \
-I$(LIBRARIES)/ArduinoCore-avr/libraries/Wire/src \
-I$(LIBRARIES)/LiquidCrystal_I2C \
-I$(LIBRARIES) \
-Iincludes \

all:
	@mkdir -p build

	# ===== C Core =====
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring.c -o build/wiring.o
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_digital.c -o build/wiring_digital.o
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_analog.c -o build/wiring_analog.o
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_pulse.c -o build/wiring_pulse.o
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_shift.c -o build/wiring_shift.o
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/WInterrupts.c -o build/WInterrupts.o

	# ===== C++ Core =====
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/Print.cpp -o build/Print.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/Stream.cpp -o build/Stream.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/HardwareSerial.cpp -o build/HardwareSerial.o
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(LIBRARIES)/ArduinoCore-avr/cores/arduino/HardwareSerial0.cpp -o build/HardwareSerial0.o

	# ===== Heizungssteuerung =====
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/main.cpp -o build/main.o

	# ===== Link =====
	@$(CXX) -mmcu=$(MCU) build/*.o -o build/main.elf
	@echo "linked to build/main.elf
	@avr-objcopy -O ihex main.elf main.hex
	@echo ".elf and .hex file created"

clean:
	@cd build && find . -name "*.o" -delete
	@echo "object files removed from build directory"
