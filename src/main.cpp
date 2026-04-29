#include "controller.h"
#include "types.h"
#include <Arduino.h>

ControllerInputData inputData; 
ControllerOutputIntent outputData; 
RealInputDevices inputDevices {inputData}; 
RealOutputDevices outputDevices {outputData}; 

SystemController controller {inputDevices, outputDevices}  ;

void setup() {controller.init(); }

void loop() { controller(); }
