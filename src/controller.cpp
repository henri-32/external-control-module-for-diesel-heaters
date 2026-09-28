#include "controller.h"
#include "interfaces.h"

SystemController::SystemController(IRuntimeConfig &c, IInputDevices &i,
                                   IOutputDevices &o)
    : runtimeConfig(c), inputDevices(i), outputDevices(o) {}

void SystemController::operator()() {
  inputDevices.update();
  applyInputdata();
  applyHeatingLogic();
  writeOutputIntent();
  outputDevices.update();
}

void SystemController::init() {
  runtimeConfig.load();
  apply_config_data();
  inputDevices.init();
  writeOutputIntent();
  outputDevices.init();
}

void SystemController::apply_config_data() {
  // {{{
  auto &target = heaterStatus.target_tempC;

  target = runtimeConfig.get_default_tempC();
  // Hier bewusst set auf sicheren Default, statt begrenzen auf Max/Min wie
  // normalerweise. Grund ist, dass zu hohe oder zu niedrige Configwerte niemals
  // hätten geschrieben werden dürfen und eher als Fehler zu betrachten sind.
  // Würde Min/Max angenommen werden, würde wahrscheinlich ein Relaisimpuls
  // ausgelöst werden, was unerwünscht ist.
  if (target < Config::kTempMinC || target > Config::kTempMaxC) {
    target = Config::kDefaultTempC;
  }
};
//}}}

void SystemController::applyInputdata() {
  //{{{
  using State = OutputDevicesIntent::LcdStateIntent;
  using Mode = HeaterStatus::Mode;
  using Command = OutputDevicesIntent::RelaisCommand;
  using LCDDirection = OutputDevicesIntent::LcdCycleDirection;
  auto &state = outputDevices.intent.lcd_state;
  auto &val = inputDevices.data.encoder_val;

  //====================================================================================
  // UIState unabhängige Eingaben
  //====================================================================================
  // Ohne gedrückten Modifier wird bei Drücken des PowerSwitches neben dem
  // Statuswechsel das Relais angefordert.
  if (inputDevices.data.switchAction.power &&
      !inputDevices.data.modifier.pressed) {
    if (heaterStatus.state == HeaterStatus::State::On) {
      heaterStatus.state = HeaterStatus::State::Off;
      heaterStatus.mode = Mode::Power;
      requestRelaisCommand(Command::Long);
      return;
    } else if (heaterStatus.state == HeaterStatus::State::Off) {
      heaterStatus.state = HeaterStatus::State::On;
      heaterStatus.mode = Mode::Power;
      requestRelaisCommand(Command::Long);
      return;
    }
  }

  // Bei gedrücktem Modifier wird nur der Status gewechselt ohne
  // RelaisBetätigung
  if (inputDevices.data.switchAction.power &&
      inputDevices.data.modifier.pressed &&
      !inputDevices.data.modifier.released) {
    if (heaterStatus.state == HeaterStatus::State::On) {
      heaterStatus.state = HeaterStatus::State::Off;
    } else if (heaterStatus.state == HeaterStatus::State::Off) {
      heaterStatus.state = HeaterStatus::State::On;
    }
    return;
  }

  // Äquivalent dazu führt das drücken des ModeSwitches bei gedrücktem
  // Modifier zum Wechsel des Modus ohne Relaisanforderung.
  // Architekturentscheidung: Das ist hier vom UIState unabhängig, da es
  // sich um eine Taste zur Fehlerbehanldung handelt und somit außerdem
  // die Switches der UI States unabhängig vom Modifier hält.
  if (inputDevices.data.switchAction.mode &&
      inputDevices.data.modifier.pressed &&
      !inputDevices.data.modifier.released) {
    if (heaterStatus.mode == Mode::Power) {
      heaterStatus.mode = Mode::Temp;
    } else if (heaterStatus.mode == Mode::Temp) {
      heaterStatus.mode = Mode::Power;
    }
    return;
  }

  // Gedrückter Modifier und Encoder wechseln die UIStates, vorausgesetzt
  // der Bildschirm ist an
  if (inputDevices.data.modifier.pressed &&
      !inputDevices.data.modifier.released && val != 0 && state != State::Off) {
    if (val >= 1 && val <= Config::kEncoderValCutoff) {
      outputDevices.intent.lcd_cycleDirection = LCDDirection::Right;
      cyclePages();
      inputDevices.data.modifier.used = true;
      return;
    }
    if (val <= -1 && val >= -Config::kEncoderValCutoff) {
      outputDevices.intent.lcd_cycleDirection = LCDDirection::Left;
      cyclePages();
      inputDevices.data.modifier.used = true;
      return;
    }
  }

  //====================================================================================
  // UI Abhängige Eingaben von ModeSwitch, DisplayButton und Encoder ohne
  // Modifier
  //====================================================================================
  switch (state) {
  case State::start_page:
    // Drücken des Modusschalters
    if (inputDevices.data.switchAction.mode) {
      if (heaterStatus.mode == Mode::Power) {
        requestRelaisCommand(Command::Short);
        heaterStatus.mode = Mode::Temp;
      } else {
        requestRelaisCommand(Command::Short);
        heaterStatus.mode = Mode::Power;
      }
      return;
    }

    // Drücken des Display Schalters
    DisplayButtonTurnsDisplayOff();

    // Encoder auswerten
    if (val == 0 || val >= Config::kEncoderValCutoff ||
        val <= -Config::kEncoderValCutoff) {
      return;
    }
    // val ist signed
    heaterStatus.target_tempC += val * Config::kTempStepC;
    clampTargetTempC(heaterStatus.target_tempC);

    break;

  case State::Page2:
    break;
  case State::Page3:
    break;
  case State::Page4:
    break;
  case State::default_temp_page:
    if (inputDevices.data.switchAction.mode) {
      pendingDefaultTempC = runtimeConfig.get_default_tempC();
      state = State::default_temp_dialog;
      return;
    }

    DisplayButtonTurnsDisplayOff();
    // Encoder hat auf dieser Keine Bedeutung
    break;

  case State::default_temp_dialog:
    // Modusschalter übernimmt den neuen default Wert
    if (inputDevices.data.switchAction.mode) {
      runtimeConfig.set_default_tempC(pendingDefaultTempC);
      state = State::default_temp_page;
      return;
    }

    // Displayschalter übernimmt den neuen default Wert nicht
    if (inputDevices.data.modifier.released &&
        !inputDevices.data.modifier.used) {
      state = State::default_temp_page;
    }

    // Bei gültigen Encoderbefehlen pending Value anpassen
    if (val == 0 || val >= Config::kEncoderValCutoff ||
        val <= -Config::kEncoderValCutoff) {
      return;
    }
    pendingDefaultTempC += val * Config::kTempStepC;
    break;

  case State::Off:
    if (inputDevices.data.modifier.released &&
        !inputDevices.data.modifier.pressed) {
      state = State::start_page;
    }
    break;

    /*
     * Deprecated
        applyModeSwitchInput();
        applyDisplayButtonInput();
        applyEncoderInput();
    */
  }
};
//}}}

void SystemController::DisplayButtonTurnsDisplayOff() {
  //{{{
  if (inputDevices.data.modifier.released && !inputDevices.data.modifier.used) {
    outputDevices.intent.lcd_state = OutputDevicesIntent::LcdStateIntent::Off;
    return;
  }
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

  // Im Dialog wird die pendingDefaultTempC angezeigt
  if (outputDevices.intent.lcd_state ==
      OutputDevicesIntent::LcdStateIntent::default_temp_dialog) {
    outputDevices.intent.displayContent.runtimeConfigData.default_tempC =
        pendingDefaultTempC;
  } else {
    outputDevices.intent.displayContent.runtimeConfigData.default_tempC =
        runtimeConfig.get_default_tempC();
  }
}
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

      // Die folgenden Seiten sind Dialoge, die nicht regulär mit
      // durchcyclen erreicht werden können.
    case LCDIntent::default_temp_dialog:
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

      // Die folgenden Seiten sind Dialoge, die nicht regulär mit
      // durchcyclen erreicht werden können.
    case LCDIntent::default_temp_dialog:
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
