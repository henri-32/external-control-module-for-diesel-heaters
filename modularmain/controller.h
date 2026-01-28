#pragma once
#include "Arduino.h"
#include "devicegroups.h"
#include "types.h"

class SystemController {
private:
  InputDevices inputDevices{inputData};
  ControllerInputData inputData;
  HeaterStatus heaterStatus;
  ControllerOutputIntent outputIntent;
  OutputDevices outputDevices{outputIntent};

public:
  SystemController() = default;

  void operator()() {
    inputDevices.updateInputData();
    applyInputdata();
    applyHeatingLogic();
    updateOutputIntent();
    outputDevices.update();
  }

  void init() {
    delay(500); // Damit sich Hardware kurz einpendeln kann
    inputDevices.init();
    outputDevices.init();
  }

private:
  void applyInputdata() {
    applyPowerSwitchInput();
    applyModeSwitchInput();
    applyEncoderInput();
  }

  void applyPowerSwitchInput() {
    /* Der Heizungsmodus wird beim OnOff Schalter immer auf POWER gewechselt, um
    die Temperaturlogik daran zu hindern, direkt zurückzuschalten. Das ersetzt
    meine alte Temperatursperre. Das ist unproblematisch weil der Modus
    unabhängig vom Zustand per modeSwitch gewechselt werden kann. Und es ist
    nötig, damit ich beim Verlassen des Bootes, die Heizung aus machen kann und
    sie korrekt herunterfährt, bevor ich den Strom wegnehme*/
    if (inputData.powerSwitchChanged) {

      if (heaterStatus.heatingState == HeaterStatus::HeatingState::ON) {
        outputIntent.requestRelaisCommand(
            ControllerOutputIntent::RelaisCommand::Long,
            ControllerOutputIntent::RelaisPriority::High);
        heaterStatus.heatingState = HeaterStatus::HeatingState::OFF;
        heaterStatus.mode = HeaterStatus::Mode::POWER;

      } else {
        outputIntent.requestRelaisCommand(
            ControllerOutputIntent::RelaisCommand::Long,
            ControllerOutputIntent::RelaisPriority::High);
        heaterStatus.heatingState = HeaterStatus::HeatingState::ON;
        heaterStatus.mode = HeaterStatus::Mode::POWER;
      }
    }
  }

  void applyModeSwitchInput() {
    if (inputData.modeSwitchChanged) {

      if (heaterStatus.mode == HeaterStatus::Mode::POWER) {
        outputIntent.relaisCommand =
            ControllerOutputIntent::RelaisCommand::Short;
        heaterStatus.mode = HeaterStatus::Mode::TEMP;
      } else {
        outputIntent.relaisCommand =
            ControllerOutputIntent::RelaisCommand::Short;
        heaterStatus.mode = HeaterStatus::Mode::POWER;
      }
    }
  }

  void applyEncoderInput() {
    constexpr float TempStep = 0.5;
    constexpr float TempMin = 5.0;
    constexpr float TempMax = 30.0;

    // Encoder Logik
    //  Hier wird der Mode geprüft, weil der Drehregler mit Knopf vllt mal die
    //  Auf die Ansicht auf Laufzeitdaten wechseln soll.
    if (inputData.encoder_val != 0) {

      if (heaterStatus.mode == HeaterStatus::Mode::TEMP) {
        heaterStatus.target_temp_c +=
            inputData.encoder_val * TempStep; // encoderVal ist signed 
        if (heaterStatus.target_temp_c > TempMax)
          heaterStatus.target_temp_c = TempMax;
        else if (heaterStatus.target_temp_c < TempMin)
          heaterStatus.target_temp_c = TempMin;
      }
    }
  }

  void applyDisplayButtonInput() {
    if (inputData.displayButtonChanged) {

      if (outputIntent.lcd_stateIntent ==
          ControllerOutputIntent::LCD_StateIntent::ON)
        outputIntent.lcd_stateIntent =
            ControllerOutputIntent::LCD_StateIntent::OFF;
      else {
        outputIntent.lcd_stateIntent =
            ControllerOutputIntent::LCD_StateIntent::ON;
      }
    }
  }

  // Temperaturregelung
  void applyHeatingLogic() {
  constexpr float setpointTolerance = 1.5;

  if (heaterStatus.mode != HeaterStatus::Mode::TEMP)
    return;
  if (inputData.sensor_tempC < heaterStatus.target_temp_c - setpointTolerance &&
      heaterStatus.heatingState == HeaterStatus::HeatingState::OFF) {
    outputIntent.requestRelaisCommand(
        ControllerOutputIntent::RelaisCommand::Long,
        ControllerOutputIntent::RelaisPriority::Low);
    heaterStatus.heatingState = HeaterStatus::HeatingState::ON;
  } else if (inputData.sensor_tempC >
                 heaterStatus.target_temp_c + setpointTolerance &&
             heaterStatus.heatingState == HeaterStatus::HeatingState::ON) {
    outputIntent.requestRelaisCommand(
        ControllerOutputIntent::RelaisCommand::Long,
        ControllerOutputIntent::RelaisPriority::Low);
    heaterStatus.heatingState = HeaterStatus::HeatingState::OFF;
  }
}

void updateOutputIntent() {
  outputIntent.displayContent.temp_c = inputData.sensor_tempC;
  outputIntent.displayContent.target_temp_c = heaterStatus.target_temp_c;
  outputIntent.displayContent.heatingState = heaterStatus.heatingState;
  outputIntent.displayContent.mode = heaterStatus.mode;
}
}
;