#pragma once
#include "devicegroups.h"
//#include "memory.h"
//#include "statistics.h"
#include "types.h"

class SystemController {
private:
  ControllerInputData inputData;
  InputDevices inputDevices{inputData};
  HeaterStatus heaterStatus;
  ControllerOutputIntent outputIntent;
  OutputDevices outputDevices{outputIntent};
  //SystemStatistics systemStatistic;
  //StatisticMemoryController memoryController;

public:
  SystemController() = default;

  void operator()() {
    inputDevices.updateInputData();
    applyInputdata();
    applyHeatingLogic();
    updateOutputIntent();
    outputDevices.update();
    //systemStatistic.update(inputData, heaterStatus);
    updateMemory();
  }

  void init() {
    outputDevices.init();
    inputDevices.init();
  }

private:
  void applyInputdata() {
    applyPowerSwitchInput();
    applyModeSwitchInput();
    applyDisplayButtonInput();
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
      if (inputData.alternatorPressed) {
        if (heaterStatus.heatingState == HeaterStatus::HeatingState::OFF)
          heaterStatus.heatingState = HeaterStatus::HeatingState::ON;
        else
          heaterStatus.heatingState = HeaterStatus::HeatingState::OFF;
        inputData.alternatorUsed = true;
        return;
      }

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
      /*Das Drücken mit Alternator ermöglicht hier das Auslösen des Relais, ohne
      den internen Modus zu wechseln, um im Fehlerfall Unterschiede zum Original
      Controller auszugleichen*/
      if (inputData.alternatorPressed) {
        if (heaterStatus.heatingState == HeaterStatus::HeatingState::OFF)
          heaterStatus.heatingState = HeaterStatus::HeatingState::ON;
        else
          heaterStatus.heatingState = HeaterStatus::HeatingState::OFF;
        inputData.alternatorUsed = true;
        return;
      }

      if (heaterStatus.mode == HeaterStatus::Mode::POWER) {
        outputIntent.requestRelaisCommand(
            ControllerOutputIntent::RelaisCommand::Short);
        heaterStatus.mode = HeaterStatus::Mode::TEMP;
      } else {
        outputIntent.requestRelaisCommand(
            ControllerOutputIntent::RelaisCommand::Short);
        heaterStatus.mode = HeaterStatus::Mode::POWER;
      }
    }
  }

  void applyEncoderInput() {
    constexpr float TempStep = 0.5;
    constexpr float TempMin = 5.0;
    constexpr float TempMax = 30.0;

    if (inputData.encoder_val == 0)
      return;
    if (inputData.alternatorPressed) {
      if (inputData.encoder_val >= 1 && inputData.encoder_val <= 6) {
        outputDevices.m_lcdDisplay.cyclePages(
            ControllerOutputIntent::LCD_CycleDirection::right);
        inputData.alternatorUsed = true;
      } else if (inputData.encoder_val <= -1 && inputData.encoder_val >= -6) {
        outputDevices.m_lcdDisplay.cyclePages(
            ControllerOutputIntent::LCD_CycleDirection::left);
        inputData.alternatorUsed = true;
      }
    }

    else {

      heaterStatus.target_temp_c +=
          inputData.encoder_val * TempStep; // encoderVal ist signed
      if (heaterStatus.target_temp_c > TempMax)
        heaterStatus.target_temp_c = TempMax;
      else if (heaterStatus.target_temp_c < TempMin)
        heaterStatus.target_temp_c = TempMin;
    }
  }

  void applyDisplayButtonInput() {
    if (inputData.displayButtonReleased) {
      if (inputData.alternatorUsed) {
        inputData.alternatorPressed = false;
        inputData.alternatorUsed = false;
        return;
      }

      if (outputIntent.lcd_stateIntent !=
          ControllerOutputIntent::LCD_StateIntent::OFF) {
        outputIntent.lcd_stateIntent =
            ControllerOutputIntent::LCD_StateIntent::OFF;
      } else {
        outputIntent.lcd_stateIntent =
            ControllerOutputIntent::LCD_StateIntent::Page1;
      }
    }
  }

  // Temperaturregelung
  void applyHeatingLogic() {
    constexpr float setpointTolerance = 1.5;

    if (heaterStatus.mode != HeaterStatus::Mode::TEMP)
      return;
    if (inputData.sensor_tempC <
            heaterStatus.target_temp_c - setpointTolerance &&
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
//    outputIntent.displayContent.runtimeDisplayData =
//        systemStatistic.getRuntimeDate();
//    outputIntent.displayContent.EEPROM_Values =
        //memoryController.getFinalAverages();
  }

  void updateMemory() {
    /*LongtimeData newLongtimeData;
    if (systemStatistic.takeLongTimeData(newLongtimeData)) {
      memoryController.update(newLongtimeData);
    }
  */}
};
