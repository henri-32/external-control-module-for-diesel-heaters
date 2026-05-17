#include "controller.h"
#include "config.h"

SystemController::SystemController(IInputDevices &i, IOutputDevices &o)
    : inputDevices(i), outputDevices(o) {}

void SystemController::operator()() {
  inputDevices.update();
  applyInputdata();
  applyHeatingLogic();
  writeOutputIntent();
  outputDevices.update();

#ifdef MEMORY_FUNCTIONS
  systemStatistic.update(inputDevices.data, heaterStatus);
  updateMemory();
#endif
}

void SystemController::init() {
  outputDevices.init();
  inputDevices.init();
}

void SystemController::applyInputdata() {
  //{{{
  applyPowerSwitchInput();
  applyModeSwitchInput();
  applyDisplayButtonInput();
  applyEncoderInput();
}
//}}}

void SystemController::applyPowerSwitchInput() {
  //{{{
  /* Der Heizungsmodus wird beim OnOff Schalter immer auf POWER gewechselt, um
  die Temperaturlogik daran zu hindern, direkt zurückzuschalten. Das ersetzt
  meine alte Temperatursperre. Das ist unproblematisch weil der Modus
  unabhängig vom Zustand per modeSwitch gewechselt werden kann. Und es ist
  nötig, damit ich beim Verlassen des Bootes, die Heizung aus machen kann und
  sie korrekt herunterfährt, bevor ich den Strom wegnehme*/
  using State = HeaterStatus::HeatingState;
  using ODI = OutputDevicesIntent;

  if (!inputDevices.data.switchAction.power) {
    return;
  };

  // Alternator Path switches only State, without relay action
  if (inputDevices.data.alternator.pressed) {
    //{{{
    if (heaterStatus.state == State::OFF) {
      heaterStatus.state = State::ON;
    } else {
      heaterStatus.state = State::OFF;
    }

    inputDevices.data.alternator.used = true;
    return;
  }
  //}}}

  if (heaterStatus.state == State::ON) {
    requestRelaisCommand(ODI::RelaisCommand::Long);
    heaterStatus.state = State::OFF;
    heaterStatus.mode = HeaterStatus::Mode::POWER;

  } else {
    requestRelaisCommand(ODI::RelaisCommand::Long);
    heaterStatus.state = State::ON;
    heaterStatus.mode = HeaterStatus::Mode::POWER;
  }
}
//}}}

void SystemController::applyModeSwitchInput() {
  //{{{
  using Mode = HeaterStatus::Mode;
  using ODI = OutputDevicesIntent;

  if (!inputDevices.data.switchAction.mode) {
    return;
  }
  // Alternator Path only switches Mode without Relay Action
  if (inputDevices.data.alternator.pressed) {
    //{{{
    if (heaterStatus.mode == Mode::POWER) {
      heaterStatus.mode = Mode::TEMP;
    } else {
      heaterStatus.mode = Mode::POWER;
    }

    inputDevices.data.alternator.used = true;
    return;
  }
  //}}}

  if (heaterStatus.mode == Mode::POWER) {
    requestRelaisCommand(ODI::RelaisCommand::Short);
    heaterStatus.mode = Mode::TEMP;
  } else {
    requestRelaisCommand(ODI::RelaisCommand::Short);
    heaterStatus.mode = Mode::POWER;
  }
}
//}}}

void SystemController::applyDisplayButtonInput() {
  //{{{
  using LCDIntent = OutputDevicesIntent::LCD_StateIntent;

  if (!inputDevices.data.alternator.released) {
    return;
  }
  // Path where alternator was used in combination with other Inputs.
  // No toggling of display expected
  if (inputDevices.data.alternator.used) {
    inputDevices.data.alternator.pressed = false;
    inputDevices.data.alternator.used = false;
    return;

    // Path without alternator for toggling display. Action happens  on
    // release of the button

  } else {
    switch (outputDevices.intent.lcd_state) {
    case LCDIntent::OFF:
      outputDevices.intent.lcd_state = LCDIntent::Page1;
      break;
    case LCDIntent::Page1:
      outputDevices.intent.lcd_state = LCDIntent::OFF;
      break;
    case LCDIntent::Page2:
      outputDevices.intent.lcd_state = LCDIntent::OFF;
      break;
    case LCDIntent::Page3:
      outputDevices.intent.lcd_state = LCDIntent::OFF;
      break;
    case LCDIntent::Page4:
      outputDevices.intent.lcd_state = LCDIntent::OFF;
    }
  }
}
//}}}

void SystemController::applyEncoderInput() {
  //{{{
  using LCDDirection = OutputDevicesIntent::LCD_CycleDirection;
  const int val = inputDevices.data.encoder_val;

  if (val == 0)
    return;

  if (inputDevices.data.alternator.pressed) {
    if (val >= 1 && val <= Config::encoderValCutoff) {
      outputDevices.intent.lcd_cycleDirection = LCDDirection::right;
      cyclePages();
      inputDevices.data.alternator.used = true;
      return;
    }
    if (val <= -1 && val >= -Config::encoderValCutoff) {
      outputDevices.intent.lcd_cycleDirection = LCDDirection::left;
      cyclePages();
      inputDevices.data.alternator.used = true;
      return;
    }
  }
  heaterStatus.target_tempC += val * Config::tempStep; // encoderVal ist signed

  // Limits to Config.h struct limits
  clampTargetTempC(heaterStatus.target_tempC);
}
//}}}

void SystemController::applyHeatingLogic() {
  //{{{
  using State = HeaterStatus::HeatingState;
  using Command = OutputDevicesIntent::RelaisCommand;

  if (heaterStatus.mode != HeaterStatus::Mode::TEMP)
    return;

  if (inputDevices.data.sensor_tempC <=
          (heaterStatus.target_tempC - Config::tolerance) &&
      heaterStatus.state == State::OFF) {

    requestRelaisCommand(Command::Long);
    heaterStatus.state = State::ON;
    return;
  }
  if (inputDevices.data.sensor_tempC >=
          (heaterStatus.target_tempC + Config::tolerance) &&
      heaterStatus.state == State::ON) {

    requestRelaisCommand(Command::Long);
    heaterStatus.state = State::OFF;
    return;
  }
}
//}}}

void SystemController::writeOutputIntent() {
  //{{{
  outputDevices.intent.displayContent.temp_c = inputDevices.data.sensor_tempC;
  outputDevices.intent.displayContent.status.target_tempC =
      heaterStatus.target_tempC;
  outputDevices.intent.displayContent.status.state = heaterStatus.state;
  outputDevices.intent.displayContent.status.mode = heaterStatus.mode;
#ifdef MEMORY_FUNCTIONS
  outputDevices.intent.displayContent.runtimeDisplayData =
      systemStatistic.getRuntimeDate();
  outputDevices.intent.displayContent.EEPROM_Values =
      memoryController.getFinalAverages();
#endif
}
//}}}

void SystemController::updateMemory() {
//{{{
#ifdef MEMORY_FUNCTIONS
  LongtimeData newLongtimeData;
  if (systemStatistic.takeLongTimeData(newLongtimeData)) {
    memoryController.update(newLongtimeData);
  }
#endif
};
//}}}

// =============Helper Functions
void SystemController::clampTargetTempC(float &target) {
  //{{{
  if (target > Config::tempMax)
    target = Config::tempMax;
  else if (target < Config::tempMin)
    target = Config::tempMin;
};
//}}}

void SystemController::cyclePages() {
  //{{{
  using LCDIntent = OutputDevicesIntent::LCD_StateIntent;

  if (outputDevices.intent.lcd_cycleDirection ==
      OutputDevicesIntent::LCD_CycleDirection::right) {
    switch (outputDevices.intent.lcd_state) {
    case LCDIntent::OFF:
      return;
    case LCDIntent::Page1:
      outputDevices.intent.lcd_state = LCDIntent::Page2;
      break;
    case LCDIntent::Page2:
      outputDevices.intent.lcd_state = LCDIntent::Page3;
      break;
    case LCDIntent::Page3:
      outputDevices.intent.lcd_state = LCDIntent::Page4;
      break;
    case LCDIntent::Page4:
      outputDevices.intent.lcd_state = LCDIntent::Page1;
      break;
    }
    return;
  }
  if (outputDevices.intent.lcd_cycleDirection ==
      OutputDevicesIntent::LCD_CycleDirection::left) {
    switch (outputDevices.intent.lcd_state) {
    case LCDIntent::OFF:
      return;
    case LCDIntent::Page1:
      outputDevices.intent.lcd_state = LCDIntent::Page3;
      break;
    case LCDIntent::Page2:
      outputDevices.intent.lcd_state = LCDIntent::Page1;
      break;
    case LCDIntent::Page3:
      outputDevices.intent.lcd_state = LCDIntent::Page2;
      break;
    case LCDIntent::Page4:
      outputDevices.intent.lcd_state = LCDIntent::Page3;
      break;
    }
    return;
  }
}

void SystemController::requestRelaisCommand(
    OutputDevicesIntent::RelaisCommand command) {
  outputDevices.intent.relaisCommand = command;
}
//}}}
