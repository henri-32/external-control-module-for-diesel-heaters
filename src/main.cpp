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
ToggleSwitch powerSwitch{my_pin_config::powerSwitch};
ToggleSwitch modeSwitch{my_pin_config::modeSwitch};
PushButton displayButton{my_pin_config::displayButton};
EncoderAdapter encoderAdapter{my_pin_config::myEncoder[0],
                              my_pin_config::myEncoder[1]};

LCDAdapter lcdAdapter{0x27, 20, 4};

RealInputDevices inputDevices{inputData, powerSwitch, modeSwitch, displayButton,
                              encoderAdapter};
RealOutputDevices outputDevices{outputData, lcdAdapter};

SystemController controller{inputDevices, outputDevices};

void setup() { controller.init(); }

void loop() { controller(); }
