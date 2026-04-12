CC  = avr-gcc
CXX = avr-g++

MCU   = atmega328p
F_CPU = 16000000UL

CFLAGS   = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os
CXXFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=c++11	-Wcpp -Os

INCLUDES = \
-Iincludes/libraries/ArduinoCore-avr/cores/arduino \
-Iincludes/libraries/ArduinoCore-avr/variants/standard

all:
	mkdir -p build

	# ===== C Core =====
	$(CC) $(CFLAGS) $(INCLUDES) -c includes/libraries/ArduinoCore-avr/cores/arduino/wiring.c -o build/wiring.o
	$(CC) $(CFLAGS) $(INCLUDES) -c includes/libraries/ArduinoCore-avr/cores/arduino/wiring_digital.c -o build/wiring_digital.o
	$(CC) $(CFLAGS) $(INCLUDES) -c includes/libraries/ArduinoCore-avr/cores/arduino/wiring_analog.c -o build/wiring_analog.o
	$(CC) $(CFLAGS) $(INCLUDES) -c includes/libraries/ArduinoCore-avr/cores/arduino/wiring_pulse.c -o build/wiring_pulse.o
	$(CC) $(CFLAGS) $(INCLUDES) -c includes/libraries/ArduinoCore-avr/cores/arduino/wiring_shift.c -o build/wiring_shift.o
	$(CC) $(CFLAGS) $(INCLUDES) -c includes/libraries/ArduinoCore-avr/cores/arduino/WInterrupts.c -o build/WInterrupts.o

	# ===== C++ Core =====
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c includes/libraries/ArduinoCore-avr/cores/arduino/Print.cpp -o build/Print.o
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c includes/libraries/ArduinoCore-avr/cores/arduino/Stream.cpp -o build/Stream.o
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c includes/libraries/ArduinoCore-avr/cores/arduino/HardwareSerial.cpp -o build/HardwareSerial.o
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c includes/libraries/ArduinoCore-avr/cores/arduino/HardwareSerial0.cpp -o build/HardwareSerial0.o

	# ===== Mein Code =====
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c includes/types.h -o build/types.o

	# ===== Link =====
	$(CXX) -mmcu=$(MCU) build/*.o -o build/main.elf
clean:
	@cd build && find . -name "*.o" -delete
	@echo "object files removed from build directory"
