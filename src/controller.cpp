#include "controller.h"
#include "interfaces.h"

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
  inputDevices.init();
  outputDevices.init();
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
  using State = HeaterStatus::State;
  using ODI = OutputDevicesIntent;

  if (!inputDevices.data.switchAction.power) {
    return;
  };

  // Alternator Path switches only State, without relay action
  if (inputDevices.data.alternator.pressed) {
    //{{{
    if (heaterStatus.state == State::Off) {
      heaterStatus.state = State::On;
    } else {
      heaterStatus.state = State::Off;
    }

    inputDevices.data.alternator.used = true;
    return;
  }
  //}}}

  if (heaterStatus.state == State::On) {
    requestRelaisCommand(ODI::RelaisCommand::Long);
    heaterStatus.state = State::Off;
    heaterStatus.mode = HeaterStatus::Mode::Power;

  } else {
    requestRelaisCommand(ODI::RelaisCommand::Long);
    heaterStatus.state = State::On;
    heaterStatus.mode = HeaterStatus::Mode::Power;
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
    if (heaterStatus.mode == Mode::Power) {
      heaterStatus.mode = Mode::Temp;
    } else {
      heaterStatus.mode = Mode::Power;
    }

    inputDevices.data.alternator.used = true;
    return;
  }
  //}}}

  if (heaterStatus.mode == Mode::Power) {
    requestRelaisCommand(ODI::RelaisCommand::Short);
    heaterStatus.mode = Mode::Temp;
  } else {
    requestRelaisCommand(ODI::RelaisCommand::Short);
    heaterStatus.mode = Mode::Power;
  }
}
//}}}

void SystemController::applyDisplayButtonInput() {
  //{{{
  using LCDIntent = OutputDevicesIntent::LcdStateIntent;

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
    case LCDIntent::Off:
      outputDevices.intent.lcd_state = LCDIntent::Page1;
      break;
    case LCDIntent::Page1:
      outputDevices.intent.lcd_state = LCDIntent::Off;
      break;
    case LCDIntent::Page2:
      outputDevices.intent.lcd_state = LCDIntent::Off;
      break;
    case LCDIntent::Page3:
      outputDevices.intent.lcd_state = LCDIntent::Off;
      break;
    case LCDIntent::Page4:
      outputDevices.intent.lcd_state = LCDIntent::Off;
    }
  }
}
//}}}

void SystemController::applyEncoderInput() {
  //{{{
  using LCDDirection = OutputDevicesIntent::LcdCycleDirection;
  const int val = inputDevices.data.encoder_val;

  if (val == 0)
    return;

  if (inputDevices.data.alternator.pressed) {
    if (val >= 1 && val <= Config::kEncoderValCutoff) {
      outputDevices.intent.lcd_cycleDirection = LCDDirection::Right;
      cyclePages();
      inputDevices.data.alternator.used = true;
      return;
    }
    if (val <= -1 && val >= -Config::kEncoderValCutoff) {
      outputDevices.intent.lcd_cycleDirection = LCDDirection::Left;
      cyclePages();
      inputDevices.data.alternator.used = true;
      return;
    }
  }
  heaterStatus.target_tempC += val * Config::kTempStepC; // encoderVal ist signed

  // Limits to Config.h struct limits
  clampTargetTempC(heaterStatus.target_tempC);
}
//}}}

void SystemController::applyHeatingLogic() {
  //{{{
  using State = HeaterStatus::State;
  using Command = OutputDevicesIntent::RelaisCommand;

  if (heaterStatus.mode != HeaterStatus::Mode::Temp)
    return;

  if (inputDevices.data.sensor_tempC <=
          (heaterStatus.target_tempC - Config::kToleranceC) &&
      heaterStatus.state == State::Off) {

    requestRelaisCommand(Command::Long);
    heaterStatus.state = State::On;
    return;
  }
  if (inputDevices.data.sensor_tempC >=
          (heaterStatus.target_tempC + Config::kToleranceC) &&
      heaterStatus.state == State::On) {

    requestRelaisCommand(Command::Long);
    heaterStatus.state = State::Off;
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
  if (target > Config::kTempMaxC)
    target = Config::kTempMaxC;
  else if (target < Config::kTempMinC)
    target = Config::kTempMinC;
};
//}}}

void SystemController::cyclePages() {
  //{{{
  using LCDIntent = OutputDevicesIntent::LcdStateIntent;

  if (outputDevices.intent.lcd_cycleDirection ==
      OutputDevicesIntent::LcdCycleDirection::Right) {
    switch (outputDevices.intent.lcd_state) {
    case LCDIntent::Off:
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
      OutputDevicesIntent::LcdCycleDirection::Left) {
    switch (outputDevices.intent.lcd_state) {
    case LCDIntent::Off:
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
