#include "controller_construction.h"
#include "config.h"
#include "devicegroups.h"
#include "display_driver.h"
#include "encoder_driver.h"
#include "library_adapter.h"
#include "pushbuttons.h"
#include "relais.h"
#include "runtime_config.h"
#include "temperature_sensor_driver.h"
#include "toggle_switches.h"
#include "types.h"

namespace {
RuntimeConfig runtimeConfig;
// Structs für die Schnittstelle des Controllers nach außen
InputDevicesDataSet inputData;
OutputDevicesIntent outputIntent;

// Hardware Konstruktion
// Pin Konfiguration findet in config.h statt
ToggleSwitch powerSwitch{PinConfig::kPowerSwitchPin};
ToggleSwitch modeSwitch{PinConfig::kModeSwitchPin};
PushButton displayButton{PinConfig::kDisplayButtonPin};
EncoderAdapter encoderHardware{PinConfig::kEncoderPinA,
                               PinConfig::kEncoderPinB};
EncoderDriver encoderDriver{encoderHardware};
OneWire one_wire{PinConfig::kTempSensorPin};
TempSensorAdapter tempSensorHardware{one_wire};
TemperatureSensorDriver tempSensorDriver{tempSensorHardware};

// Absichtlich hardcoded, da Display Hardware im aktuellen
// System sowieso nicht sinnvoll getauscht werden kann.
LCDAdapter lcdAdapter{0x27, 20, 4};
DisplayDriver displayDriver{lcdAdapter};

Relais relais{PinConfig::kRelaisPin};

InputDevices inputDevices{inputData,     powerSwitch,   modeSwitch,
                          displayButton, encoderDriver, tempSensorDriver};
OutputDevices outputDevices{outputIntent, displayDriver, relais};
} // namespace

SystemController controller{runtimeConfig, inputDevices, outputDevices};
