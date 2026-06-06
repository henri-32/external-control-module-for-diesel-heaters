#include "controller_construction.h"
#include "config.h"
#include "devicegroups.h"
#include "display_driver.h"
#include "encoder_driver.h"
#include "library_adapter.h"
#include "pushbuttons.h"
#include "temperature_sensor_driver.h"
#include "toggle_switches.h"
#include "relais.h"
#include "types.h"

namespace {
// Structs für die Schnittstelle des Controllers nach außen
InputDevicesDataSet inputData;
OutputDevicesIntent outputIntent;

// Hardware Konstruktion
ToggleSwitch powerSwitch{PinConfig::kPowerSwitchPin};
ToggleSwitch modeSwitch{PinConfig::kModeSwitchPin};
PushButton displayButton{PinConfig::kDisplayButtonPin};
EncoderAdapter encoderHardware{PinConfig::kEncoderPinA,
                               PinConfig::kEncoderPinB};
EncoderDriver encoderDriver{encoderHardware};
OneWire one_wire{PinConfig::kTempSensorPin};
TempSensorAdapter tempSensorHardware{one_wire};
TemperatureSensorDriver tempSensorDriver{tempSensorHardware};

LCDAdapter lcdAdapter{0x27, 20, 4};
DisplayDriver displayDriver{lcdAdapter, outputIntent.displayContent,
                            outputIntent.lcd_state};

Relais relais{PinConfig::kRelaisPin};

InputDevices inputDevices{inputData, powerSwitch, modeSwitch, displayButton,
                          encoderDriver, tempSensorDriver};
OutputDevices outputDevices{outputIntent, displayDriver, relais};
}

SystemController controller{inputDevices, outputDevices};

