#include "controller.h"
#include "library_adapter.h"
#include "config.h"
#include "devicegroups.h"
#include "toggle_switches.h"
#include "pushbuttons.h"
#include "types.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// Structs für die Schnittstelle des Controllers nach außen
ControllerInputData inputData;
ControllerOutputIntent outputData;

// Hardware Konstruktion
ToggleSwitch powerSwitch{pinConfig::powerSwitch};
ToggleSwitch modeSwitch{pinConfig::modeSwitch};
PushButton displayButton{pinConfig::displayButton};
EncoderAdapter encoder{pinConfig::myEncoder[0],
                              pinConfig::myEncoder[1]};
OneWire one_wire {pinConfig::tempSensor}; 
TempSensorAdapter sensor {one_wire}; 



LCDAdapter lcdAdapter{0x27, 20, 4};
Relais relais{pinConfig::relais}; 


InputDevices inputDevices{inputData, powerSwitch, modeSwitch, displayButton,
                              encoder, sensor};
OutputDevices outputDevices{outputData, lcdAdapter, relais};

SystemController controller{inputDevices, outputDevices};

void setup() { controller.init(); }

void loop() { controller(); }
