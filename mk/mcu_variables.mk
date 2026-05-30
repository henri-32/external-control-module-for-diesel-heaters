include mk/globals.mk
INCLUDES := \
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
	-I$(LIBRARIES) \
	-Iincludes

AVR_C_SRCS := \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring.c \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_digital.c \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_analog.c \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_pulse.c \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/wiring_shift.c \
	$(LIBRARIES)/ArduinoCore-avr/libraries/Wire/src/utility/twi.c \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/WInterrupts.c

AVR_CPP_SRCS := \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/Print.cpp \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/Stream.cpp \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/WString.cpp \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/HardwareSerial.cpp \
	$(LIBRARIES)/ArduinoCore-avr/libraries/SoftwareSerial/src/SoftwareSerial.cpp \
	$(LIBRARIES)/ArduinoCore-avr/libraries/Wire/src/Wire.cpp \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/HardwareSerial0.cpp \
	$(LIBRARIES)/ArduinoCore-avr/cores/arduino/main.cpp \
	$(LIBRARIES)/LiquidCrystal_I2C/LiquidCrystal_I2C.cpp \
	$(LIBRARIES)/OneWire/OneWire.cpp \
	$(LIBRARIES)/Arduino-Temperature-Control-Library/DallasTemperature.cpp \
	$(LIBRARIES)/Encoder/Encoder.cpp

APP_CPP_SRCS := \
	src/linker_stubs.cpp \
	src/main.cpp \
	src/controller.cpp \
	src/controller_construction.cpp \
	src/devicegroups.cpp \
	src/display_driver.cpp \
	src/relais.cpp \
	src/pushbuttons.cpp \
	src/toggle_switches.cpp \
	src/encoder_driver.cpp \
	src/temperature_sensor_driver.cpp \
	src/library_adapter.cpp


MCU_CPPFLAGS := -DF_CPU=$(F_CPU) -DARDUINO=10800 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR -DDECIMAL_DIG=__DECIMAL_DIG__
MCU_OPT := -Os -ffunction-sections -fdata-sections

C_OBJS := $(addprefix $(MCU_BUILD_DIR)/,$(AVR_C_SRCS:.c=.o))
CPP_OBJS := $(addprefix $(MCU_BUILD_DIR)/,$(AVR_CPP_SRCS:.cpp=.o) $(APP_CPP_SRCS:.cpp=.o))
MCU_OBJS := $(C_OBJS) $(CPP_OBJS)

MCU_DEPS := $(MCU_OBJS:.o=.d)

CFLAGS := -mmcu=$(MCU) -std=gnu11 $(MCU_OPT) -MMD -MP
CXXFLAGS := -mmcu=$(MCU) -std=gnu++11 -Wcpp $(MCU_OPT) -fno-exceptions -fno-rtti -MMD -MP

