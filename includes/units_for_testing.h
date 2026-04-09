/* In dieser Datei wurden private Methoden aus verschiedenen Klassen extrahiert, um sie testbar zu machen. Die Testdatei inkludiert nur diese Datei, sodass hier die Abhängigkeiten aufgelöst werden und die Testumgebung unabhängig vom Produktivcode ist.
Die enthaltene Logik hier ist identisch aus den Klassen übernommen 
*/

#include "types.h"
#include "devicegroups.h"


HeaterStatus heaterStatus; 
ControllerInputData inputData; 
ControllerOutputIntent outputIntent;
OutputDevices outputDevices{outputIntent};

  void applyModeSwitchInput() {
    if (inputData.modeSwitchChanged) {

      if (heaterStatus.mode == HeaterStatus::Mode::POWER) {
        outputDevices.requestRelaisAction(
            ControllerOutputIntent::RelaisCommand::Short);
        heaterStatus.mode = HeaterStatus::Mode::TEMP;
      } else {
        outputDevices.requestRelaisAction(
            ControllerOutputIntent::RelaisCommand::Short);
        heaterStatus.mode = HeaterStatus::Mode::POWER;
      }
    }
  }

  void applyPowerSwitchInput() {
    if (inputData.powerSwitchChanged) {

      if (heaterStatus.heatingState == HeaterStatus::HeatingState::ON) {
        outputDevices.requestRelaisAction(
            ControllerOutputIntent::RelaisCommand::Long,
            ControllerOutputIntent::RelaisPriority::High);
        heaterStatus.heatingState = HeaterStatus::HeatingState::OFF;
        heaterStatus.mode = HeaterStatus::Mode::POWER;

      } else {
        outputDevices.requestRelaisAction(
            ControllerOutputIntent::RelaisCommand::Long,
            ControllerOutputIntent::RelaisPriority::High);
        heaterStatus.heatingState = HeaterStatus::HeatingState::ON;
        heaterStatus.mode = HeaterStatus::Mode::POWER;
      }
    }
  }

