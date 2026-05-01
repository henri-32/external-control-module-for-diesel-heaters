#include "controller.h"
#include "types.h"
#include "library_adapter.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

//Structs für die Schnittstelle des Controllers nach außen 
ControllerInputData inputData; 
ControllerOutputIntent outputData; 

//Hardware Treiber
LCDAdapter lcdAdapter {0x27, 20, 4}; 


RealInputDevices inputDevices {inputData}; 
RealOutputDevices outputDevices {outputData, lcdAdapter}; 

SystemController controller {inputDevices, outputDevices}  ;

void setup() {controller.init(); }

void loop() { controller(); }
