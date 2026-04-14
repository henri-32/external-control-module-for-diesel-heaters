#include "controller.h"

void SystemController::operator()() {
  inputDevices.updateInputData();
  applyInputdata();
  applyHeatingLogic();
  updateOutputIntent();
  outputDevices.update();
  // systemStatistic.update(inputData, heaterStatus);
  updateMemory();
}

void SystemController::init() {
  outputDevices.init();
  inputDevices.init();
}

void SystemController::applyInputdata() {
  applyPowerSwitchInput();
  applyModeSwitchInput();
  applyDisplayButtonInput();
  applyEncoderInput();
}

void SystemController::applyPowerSwitchInput() {
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

void SystemController::applyModeSwitchInput() {
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

void SystemController::applyEncoderInput() {
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
}

void SystemController::applyDisplayButtonInput() {
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

void SystemController::applyHeatingLogic() {
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

void SystemController::updateOutputIntent() {
  outputIntent.displayContent.temp_c = inputData.sensor_tempC;
  outputIntent.displayContent.target_temp_c = heaterStatus.target_temp_c;
  outputIntent.displayContent.heatingState = heaterStatus.heatingState;
  outputIntent.displayContent.mode = heaterStatus.mode;
#ifdef MEMORY_FUNCTIONS
  outputIntent.displayContent.runtimeDisplayData =
      systemStatistic.getRuntimeDate();
  outputIntent.displayContent.EEPROM_Values =
      memoryController.getFinalAverages();
#endif
}

void SystemController::updateMemory() {
#ifdef MEMORY_FUNCTIONS
  LongtimeData newLongtimeData;
  if (systemStatistic.takeLongTimeData(newLongtimeData)) {
    memoryController.update(newLongtimeData);
  }
#endif
};
