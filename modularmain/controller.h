#pragma once
#include "types.h"
#include "devicegroups.h"

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
    inputDevices.pollAndWriteControllerInputData();
    applyInputdata();
    applyHeatingLogic();
    writeStatesToOutputIntent();
    outputDevices.update();
  }

  void init() {
    delay(500); // Damit sich Hardware kurz einpendeln kann
    inputDevices.init();
    outputDevices.init();
  }

private:
  void applyInputdata() {;
    /* Der Heizungsmodus wird beim OnOff Schalter immer auf POWER gewechselt, um
    die Temperaturlogik daran zu hindern, direkt zurückzuschalten. Das ersetzt
    meine alte Temperatursperre. Das ist unproblematisch weil der Modus
    unabhängig vom Zustand per modeSwitch gewechselt werden kann. Und es ist
    nötig, damit ich beim Verlassen des Bootes, die Heizung aus machen kann und
    sie korrekt herunterfährt, bevor ich den Strom wegnehme*/
    constexpr float TempStep = 0.5;
    constexpr float TempMin = 5.0;
    constexpr float TempMax = 30.0;

    // Power Switch Logik
    if (inputData.powerSwitchChanged) {

      if (heaterStatus.heatingState == HeaterStatus::HeatingState::ON) {
        outputIntent.relaisCommand =
            ControllerOutputIntent::RelaisCommand::Long;
        heaterStatus.heatingState = HeaterStatus::HeatingState::OFF;
        heaterStatus.mode = HeaterStatus::Mode::POWER;

      } else {
        outputIntent.relaisCommand =
            ControllerOutputIntent::RelaisCommand::Long;
        heaterStatus.heatingState = HeaterStatus::HeatingState::ON;
        heaterStatus.mode = HeaterStatus::Mode::POWER;
      }
    }

    // Mode Switch Logik
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

    // Encoder Logik
    //  Hier wird der Modus geprüft, weil der Drehregler mit Knopf vllt mal die
    //  Ansicht auf Laufzeitdaten wechseln soll.
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

    // Display Switch Logik
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
    constexpr float Solltoleranz = 1.5;

    if (heaterStatus.mode != HeaterStatus::Mode::TEMP)
      return;
    if (inputData.sensor_tempC < heaterStatus.target_temp_c - Solltoleranz &&
        heaterStatus.heatingState == HeaterStatus::HeatingState::OFF) {
      outputIntent.relaisCommand = ControllerOutputIntent::RelaisCommand::Long;
      heaterStatus.heatingState = HeaterStatus::HeatingState::ON;
    } else if (inputData.sensor_tempC >
                   heaterStatus.target_temp_c + Solltoleranz &&
               heaterStatus.heatingState == HeaterStatus::HeatingState::ON) {
      outputIntent.relaisCommand = ControllerOutputIntent::RelaisCommand::Long;
      heaterStatus.heatingState = HeaterStatus::HeatingState::OFF;
    }
  }

  void writeStatesToOutputIntent() {
    outputIntent.displayContent.temp_c = inputData.sensor_tempC;
    outputIntent.displayContent.target_temp_c = heaterStatus.target_temp_c;
    outputIntent.displayContent.heatingState = heaterStatus.heatingState;
    outputIntent.displayContent.mode = heaterStatus.mode;
  }
};