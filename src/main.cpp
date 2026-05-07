#include "config.h"
#include "controller.h"
#include "devicegroups.h"
#include "display_driver.h"
#include "encoder_driver.h"
#include "library_adapter.h"
#include "pushbuttons.h"
#include "temperature_sensor_driver.h"
#include "toggle_switches.h"
#include "relais.h"
#include "types.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// Structs für die Schnittstelle des Controllers nach außen
ControllerInputData inputData;
ControllerOutputIntent outputIntent;

// Hardware Konstruktion
ToggleSwitch powerSwitch{pinConfig::powerSwitch};
ToggleSwitch modeSwitch{pinConfig::modeSwitch};
PushButton displayButton{pinConfig::displayButton};
EncoderAdapter encoderHardware{pinConfig::myEncoder[0],
                               pinConfig::myEncoder[1]};
EncoderDriver encoderDriver{encoderHardware};
OneWire one_wire{pinConfig::tempSensor};
TempSensorAdapter tempSensorHardware{one_wire};
TemperatureSensorDriver tempSensorDriver{tempSensorHardware};

LCDAdapter lcdAdapter{0x27, 20, 4};
DisplayDriver displayDriver{lcdAdapter, outputIntent.displayContent,
                            outputIntent.lcd_stateIntent};

Relais relais{pinConfig::relais};

InputDevices inputDevices{inputData, powerSwitch, modeSwitch, displayButton,
                          encoderDriver, tempSensorDriver};
OutputDevices outputDevices{outputIntent, displayDriver, relais};

SystemController controller{inputDevices, outputDevices};

void setup() { controller.init(); }

void loop() { controller(); }
