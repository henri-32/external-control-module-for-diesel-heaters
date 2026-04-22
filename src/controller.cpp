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
  using State = HeaterStatus::HeatingState;
  using COI = ControllerOutputIntent;

  if (!inputData.powerSwitchChanged) {
    return;
  };

  if (inputData.alternatorPressed) {
    if (heaterStatus.heatingState == State::OFF) {
      heaterStatus.heatingState = State::ON;
    } else {
      heaterStatus.heatingState = State::OFF;
    }

    inputData.alternatorUsed = true;
    return;
  }

  if (heaterStatus.heatingState == State::ON) {
    outputIntent.requestRelaisCommand(COI::RelaisCommand::Long,
                                      COI::RelaisPriority::High);
    heaterStatus.heatingState = State::OFF;
    heaterStatus.mode = HeaterStatus::Mode::POWER;

  } else {
    outputIntent.requestRelaisCommand(COI::RelaisCommand::Long,
                                      COI::RelaisPriority::High);
    heaterStatus.heatingState = State::ON;
    heaterStatus.mode = HeaterStatus::Mode::POWER;
  }
}

void SystemController::applyModeSwitchInput() {
  //{{{
  using Mode = HeaterStatus::Mode;
  using COI = ControllerOutputIntent;

  if (!inputData.modeSwitchChanged) {
    return;
  }
  // Alternator Path only switches Mode without Relay Action
  if (inputData.alternatorPressed) {
    //{{{
    if (heaterStatus.mode == Mode::POWER) {
	  heaterStatus.mode = Mode::TEMP;
    } else {
	  heaterStatus.mode = Mode::POWER;	
    }

    inputData.alternatorUsed = true;
    return;
  }
  //}}}

  if (heaterStatus.mode == Mode::POWER) {
    outputIntent.requestRelaisCommand(COI::RelaisCommand::Short);
    heaterStatus.mode = Mode::TEMP;
  } else {
    outputIntent.requestRelaisCommand(COI::RelaisCommand::Short);
    heaterStatus.mode = Mode::POWER;
  }
}
//}}}

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
  if (inputData.alternatorReleased) {
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
