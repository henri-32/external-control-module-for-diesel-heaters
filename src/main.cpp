#include "controller.h"
#include "types.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

ControllerInputData inputData; 
ControllerOutputIntent outputData; 
LiquidCrystal_I2C lcdLibObject{0x27, 20, 4};


RealInputDevices inputDevices {inputData}; 
RealOutputDevices outputDevices {outputData, lcdLibObject}; 

SystemController controller {inputDevices, outputDevices}  ;

void setup() {controller.init(); }

void loop() { controller(); }
