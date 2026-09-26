#include "controller.h"
#include "interfaces.h"

SystemController::SystemController(IModifiableConfig &c, IInputDevices &i,
                                   IOutputDevices &o)
    : modifiableConfig(c), inputDevices(i), outputDevices(o) {}

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
  modifiableConfig.load();
  apply_config_data();
  inputDevices.init();
  outputDevices.init();
}

void SystemController::apply_config_data() {
  // {{{

  heaterStatus.target_tempC = modifiableConfig.get_default_tempC();
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
    DisplayButtonForDisplayOff();

    // Encoder auswerten
    if (val == 0 || val >= Config::kEncoderValCutoff) {
      return;
    }
    // val ist signed
    heaterStatus.target_tempC += val * Config::kTempStepC;

    break;

  case State::Page2:
    break;
  case State::Page3:
    break;
  case State::Page4:
    break;
  case State::default_temp_page:
    if (inputDevices.data.switchAction.mode) {
      pendingDefaultTempC = modifiableConfig.get_default_tempC();
      state = State::default_temp_dialog;
      return;
    }

    DisplayButtonForDisplayOff();
    // Encoder hat auf dieser Keine Bedeutung
    break;

  case State::default_temp_dialog:
    // Modusschalter übernimmt den neuen default Wert
    if (inputDevices.data.switchAction.mode) {
      modifiableConfig.set_default_tempC(pendingDefaultTempC);
      state = State::default_temp_page;
      return;
    }

    // Displayschalter übernimmt den neuen default Wert nicht
    if (inputDevices.data.modifier.released &&
        !inputDevices.data.modifier.used) {
      state = State::default_temp_page;
    }

    // Bei gültigen Encoderbefehlen pending Value anpassen
    if (val == 0 || val >= Config::kEncoderValCutoff) {
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

void SystemController::DisplayButtonForDisplayOff() {
  //{{{
  if (inputDevices.data.modifier.released && !inputDevices.data.modifier.used) {
    outputDevices.intent.lcd_state = OutputDevicesIntent::LcdStateIntent::Off;
    return;
  }
}
//}}}
/*
 * Deprecated
void SystemController::applyPowerSwitchInput() {
  //{{{
  * Der Heizungsmodus wird beim OnOff Schalter immer auf POWER gewechselt,
um die Temperaturlogik daran zu hindern, direkt zurückzuschalten. Das
ersetzt meine alte Temperatursperre. Das ist unproblematisch weil der Modus
  unabhängig vom Zustand per modeSwitch gewechselt werden kann. Und es ist
  nötig, damit ich beim Verlassen des Bootes, die Heizung aus machen kann
und sie korrekt herunterfährt, bevor ich den Strom wegnehme* using State =
HeaterStatus::State; using ODI = OutputDevicesIntent;

  if (!inputDevices.data.switchAction.power) {
    return;
  };

  // Pfad mit gedrücktem Modifier. Status ON/OFF wird gewechselt ohne
  // Relaisbetätigung
  if (inputDevices.data.modifier.pressed) {
    //{{{
    if (heaterStatus.state == State::Off) {
      heaterStatus.state = State::On;
    } else {
      heaterStatus.state = State::Off;
    }

    inputDevices.data.modifier.used = true;
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
  // Bei gedrücktem Modifier wird nur der Modus gewechselt, ohne Betätigung
  // des Relais
  if (inputDevices.data.modifier.pressed) {
    //{{{
    if (heaterStatus.mode == Mode::Power) {
      heaterStatus.mode = Mode::Temp;
    } else {
      heaterStatus.mode = Mode::Power;
    }

    inputDevices.data.modifier.used = true;
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
    dataBuffer.current_default_target_tempC =
        modifiableConfig.get_default_tempC();
    dataBuffer.intended_default_target_tempC =
        modifiableConfig.get_default_tempC();
    outputDevices.intent.lcd_state =
ODI::LcdStateIntent::default_temp_dialog; break; case
ODI::LcdStateIntent::default_temp_dialog:
    modifiableConfig.set_default_tempC(
        dataBuffer.intended_default_target_tempC);
    outputDevices.intent.lcd_state = ODI::LcdStateIntent::default_temp_page;
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

  if (!inputDevices.data.modifier.released) {
    return;
  }
  // Path wo der modifier in Kombination genutzt wurde.
  // Kein Toggle des Displays erwartet.
  if (inputDevices.data.modifier.used) {
    inputDevices.data.modifier.pressed = false;
    inputDevices.data.modifier.used = false;
    return;

    // Path ohne modifier Kombination zum Display Toggle.
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

    case LCDIntent::default_temp_dialog:
      modifiableConfig.set_default_tempC(
          dataBuffer.current_default_target_tempC);
      outputDevices.intent.lcd_state = LCDIntent::default_temp_page;
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

  if (inputDevices.data.modifier.pressed) {
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
  // Pfad ohne Modifier
  // val auf in Config vorgeschriebende Werte begrenzen
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
    clampTargetTempC(heaterStatus.target_tempC);
    break;
  case LCDIntent::Page2:
    break;
  case LCDIntent::Page3:
    break;
  case LCDIntent::Page4:
    break;
  case LCDIntent::default_temp_page:
    break;
  case LCDIntent::default_temp_dialog:
    dataBuffer.intended_default_target_tempC += val * Config::kTempStepC;
    clampTargetTempC(dataBuffer.intended_default_target_tempC);
    break;
  case LCDIntent::Off:
    break;

  default:
    break;
  };
}
//}}}
//
*/

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
  if (outputDevices.intent.lcd_state ==
      OutputDevicesIntent::LcdStateIntent::default_temp_dialog) {
    outputDevices.intent.displayContent.modifiableConfigData.default_tempC =
        pendingDefaultTempC;
  } else {
    outputDevices.intent.displayContent.modifiableConfigData.default_tempC =
        modifiableConfig.get_default_tempC();
  }
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
