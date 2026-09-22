#include "controller.h"
#include "interfaces.h"
#include <stdio.h>

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

  // Pfad mit gedrücktem Modifier. Status ON/OFF wird gewechselt ohne
  // Relaisbetätigung
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
  // Bei gedrücktem Modifier wird nur der Modus gewechselt, ohne Betätigung des
  // Relais
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

  switch (outputDevices.intent.lcd_state) {
  case ODI::LcdStateIntent::start_page:
    if (heaterStatus.mode == Mode::Power) {
      requestRelaisCommand(ODI::RelaisCommand::Short);
      heaterStatus.mode = Mode::Temp;
    } else {
      requestRelaisCommand(ODI::RelaisCommand::Short);
      heaterStatus.mode = Mode::Power;
    }
    break;
  case ODI::LcdStateIntent::Page2:
    break;
  case ODI::LcdStateIntent::Page3:
    break;
  case ODI::LcdStateIntent::Page4:
    break;
  case ODI::LcdStateIntent::default_temp_page:
    enter_default_temp_dialog();
    break;
  case ODI::LcdStateIntent::Off:
    break;
  default:
    break;
  }
}
//}}}

void SystemController::applyDisplayButtonInput() {
  //{{{
  using LCDIntent = OutputDevicesIntent::LcdStateIntent;

  if (!inputDevices.data.alternator.released) {
    return;
  }
  // Path wo der alternator in Kombination genutzt wurde.
  // Kein Toggle des Displays erwartet.
  if (inputDevices.data.alternator.used) {
    inputDevices.data.alternator.pressed = false;
    inputDevices.data.alternator.used = false;
    return;

    // Path ohne alternator Kombination zum Display Toggle.
    // Action on release

  } else {
    switch (outputDevices.intent.lcd_state) {
    case LCDIntent::Off:
      outputDevices.intent.lcd_state = LCDIntent::start_page;
      break;
    case LCDIntent::start_page:
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
      break;
    case LCDIntent::default_temp_page:
      outputDevices.intent.lcd_state = LCDIntent::Off;
      break;
    }
  }
}
//}}}

void SystemController::applyEncoderInput() {
  //{{{
  using LCDDirection = OutputDevicesIntent::LcdCycleDirection;
  int val = inputDevices.data.encoder_val;

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
  //val auf in Config vorgeschriebende Werte begrenzen 
  if (val > Config::kEncoderValCutoff) {
    val = Config::kEncoderValCutoff;
  } else if (val < -Config::kEncoderValCutoff) {
    val = -Config::kEncoderValCutoff;
  }

  using LCDIntent = OutputDevicesIntent::LcdStateIntent;
  switch (outputDevices.intent.lcd_state) {
  case LCDIntent::start_page:
    heaterStatus.target_tempC +=
        val * Config::kTempStepC; // encoderVal ist signed
    break;
  case LCDIntent::Page2:
    break;
  case LCDIntent::Page3:
    break;
  case LCDIntent::Page4:
    break;
  case LCDIntent::default_temp_page:
    heaterStatus.default_target_tempC += val * Config::kTempStepC;
    break;
  case LCDIntent::Off:
    break;

  default:
    break;
  };

  // Begrenzt auf in config.h festgelegte zulässige Werte
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
    case LCDIntent::start_page:
      outputDevices.intent.lcd_state = LCDIntent::Page2;
      break;
    case LCDIntent::Page2:
      outputDevices.intent.lcd_state = LCDIntent::Page3;
      break;
    case LCDIntent::Page3:
      outputDevices.intent.lcd_state = LCDIntent::Page4;
      break;
    case LCDIntent::Page4:
      outputDevices.intent.lcd_state = LCDIntent::default_temp_page;
      break;

    case LCDIntent::default_temp_page:
      outputDevices.intent.lcd_state = LCDIntent::start_page;
      break;
    }
    return;
  }
  if (outputDevices.intent.lcd_cycleDirection ==
      OutputDevicesIntent::LcdCycleDirection::Left) {
    switch (outputDevices.intent.lcd_state) {
    case LCDIntent::Off:
      return;
    case LCDIntent::start_page:
      outputDevices.intent.lcd_state = LCDIntent::default_temp_page;
      break;
    case LCDIntent::Page2:
      outputDevices.intent.lcd_state = LCDIntent::start_page;
      break;
    case LCDIntent::Page3:
      outputDevices.intent.lcd_state = LCDIntent::Page2;
      break;
    case LCDIntent::Page4:
      outputDevices.intent.lcd_state = LCDIntent::Page3;
      break;
    case LCDIntent::default_temp_page:
      outputDevices.intent.lcd_state = LCDIntent::Page4;
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

void SystemController::enter_default_temp_dialog() {
  // lokale Variable für display
  float default_temp_buffer = heaterStatus.default_target_tempC;

  while (true) {
    char lines[4][21];
    snprintf(lines[0], 21, "default temp %.2f", default_temp_buffer);
    snprintf(lines[1], 21, "Display Button: OK");
    snprintf(lines[2], 21, "Mode Button: EXIT");

    // Encoder auswerten
    const int val = inputDevices.data.encoder_val;
    if (val == 0) {
      continue;
    }

    default_temp_buffer += val * Config::kTempStepC; // val ist signed
  }
}
