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
  // Für jeden Tick lokal in dieser Funktion zurücksetzen
  modifier_used_ = false; 

  using State = UiState;
  using Mode = HeaterStatus::Mode;
  using Command = OutputDevicesIntent::RelaisCommand;
  using LCDDirection = OutputDevicesIntent::LcdCycleDirection;
  auto &id = inputDevices.data;
  auto &oi = outputDevices.intent;
  auto &state = heaterStatus.uiState;
  auto &val = inputDevices.data.encoder_val;

  /* Die Bedienlogik liegt hier implizit in der Platzierung der Returns und der
   * Priorisierung der Funktionen. Gleichzeitige Eingaben mehrerer Schalter
   * werden so implizit, aber "durchdacht" priorisiert.  Zum jetzigen Zeitpunkt
   * rechtfertigt der begrenzte UI Funktionsumfang keine eigene
   * Abstraktionsebene bzw. ein unabhängiges Input Modell.
   */

  //====================================================================================
  // UIState unabhängige Eingaben
  //====================================================================================

  // Ohne gedrückten Modifier wird bei Drücken des PowerSwitches neben dem
  // Statuswechsel das Relais angefordert.
  if (id.switchAction.power && !id.modifier.pressed) {
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
  if (id.switchAction.power && id.modifier.pressed && !id.modifier.released) {
    if (heaterStatus.state == HeaterStatus::State::On) {
      heaterStatus.state = HeaterStatus::State::Off;
    } else if (heaterStatus.state == HeaterStatus::State::Off) {
      heaterStatus.state = HeaterStatus::State::On;
    }
    modifier_used_ = true;
    return;
  }

  // Äquivalent dazu führt das drücken des ModeSwitches bei gedrücktem
  // Modifier zum Wechsel des Modus ohne Relaisanforderung.

  // Architekturentscheidung: Das ist hier vom UIState unabhängig, da es
  // sich um eine Taste zur Fehlerbehandlung handelt
  if (id.switchAction.mode && id.modifier.pressed && !id.modifier.released) {
    if (heaterStatus.mode == Mode::Power) {
      heaterStatus.mode = Mode::Temp;
    } else if (heaterStatus.mode == Mode::Temp) {
      heaterStatus.mode = Mode::Power;
    }
    modifier_used_ = true;
    return;
  }

  // Gedrückter Modifier und Encoder wechseln die UIStates, vorausgesetzt
  // der Bildschirm ist angeschaltet
  if (id.modifier.pressed && !id.modifier.released && val != 0 &&
      state != State::Off) {
    if (val >= 1 && val <= Config::kEncoderValCutoff) {
      oi.lcd_cycleDirection = LCDDirection::Right;
      cyclePages();
      modifier_used_ = true;
      return;
    }
    if (val <= -1 && val >= -Config::kEncoderValCutoff) {
      oi.lcd_cycleDirection = LCDDirection::Left;
      cyclePages();
      modifier_used_ = true;
      return;
    }
  }

  //====================================================================================
  // UIState abhängige Eingaben von ModeSwitch, DisplayButton und Encoder ohne
  // Modifier
  //====================================================================================
  switch (state) {
  case State::start_page:

    // Auf der Startseite wechselt der Modusschalter den Temperaturmodus der
    // Heizung
    if (id.switchAction.mode) {
      if (heaterStatus.mode == Mode::Power) {
        requestRelaisCommand(Command::Short);
        heaterStatus.mode = Mode::Temp;
      } else {
        requestRelaisCommand(Command::Short);
        heaterStatus.mode = Mode::Power;
      }
      return;
    }

	//DisplayButton schaltet Display aus.
    if (displayButtonAction()) {
      state = State::Off;
    }

    // Der Encoder verstellt die Solltemperatur
    if (val == 0 || val >= Config::kEncoderValCutoff ||
        val <= -Config::kEncoderValCutoff) {
      return;
    }
    // val ist signed
    heaterStatus.target_tempC += val * Config::kTempStepC;
    clampTempToConfigVals(heaterStatus.target_tempC);
    break;

  case State::menu_page:
    if (id.switchAction.mode) {
      state = State::default_temp_menu;
      return;
    }
    break;

  case State::default_temp_menu:
    // Die Seite für die default_temp bietet einen Dialog an, welcher durch
    // drücken des Modusschalters geöffnet wird (anderer UIState)
    if (id.switchAction.mode) {
      pendingDefaultTempC_ = runtimeConfig.get_default_tempC();
      state = State::default_temp_dialog;
      return;

      // Der Display Button geht zurück zur Menü Seite
      if (displayButtonAction()) {
        state = State::menu_page;
      }

      if (id.encoder_val < 0) { // TODO Cycle Menu Pages}
        if (id.encoder_val > 0) {
        }
        break;

      case State::short_relais_impulse_menu:
        if (id.switchAction.mode) {
          state = State::pulse_dialog;
        }

        if (displayButtonAction()) {
          state = State::menu_page;
        }

        if (id.encoder_val < 0) {
        }
        if (id.encoder_val > 0) {
        }
        break;

      case State::default_temp_dialog:
        /*Im geöffneten default_temp Dialog wird durch den Encoder ein
         * pendingDefault Wert verstellt. Der Modusschalter übernimmt diesen,
         * der DisplayButton verwirft ihn und kehrt zum vorherigen default_temp
         * UIState zurück.
         */

        // Übernahme des pendingDefault Werts
        if (id.switchAction.mode) {
          runtimeConfig.set_default_tempC(pendingDefaultTempC_);
          state = State::menu_page;
          return;
        }

        // Keine Übernahme
        if (id.modifier.released) {
          state = State::default_temp_menu;
        }

        // Bei gültigen Encoderbefehlen pending Value anpassen
        if (val == 0 || val >= Config::kEncoderValCutoff ||
            val <= -Config::kEncoderValCutoff) {
          return;
        }
        pendingDefaultTempC_ += val * Config::kTempStepC;
        clampTempToConfigVals(pendingDefaultTempC_);
        break;

      case State::pulse_dialog:
        break;

      case State::Off:
        // Bei ausgeschaltetem Display hat nur der DisplayButton Funtion und
        // schaltet das Display ein.
        if (id.modifier.released && !id.modifier.pressed) {
          state = State::start_page;
        }
        break;
      }
    }
  }
};
//}}}


void SystemController::applyHeatingLogic() {
  //{{{
  using State = HeaterStatus::State;
  using Command = OutputDevicesIntent::RelaisCommand;

  // Heizungslogik greift nur im Temperaturmodus.
  // -127 und 85 sind Fehler bzw. uninitialisierte Startwerte des DS18B20
  // Sensors und sollen keine Heizimpulse auslösen
  if (heaterStatus.mode != HeaterStatus::Mode::Temp ||
      inputDevices.data.sensor_tempC == -127.0 ||
      inputDevices.data.sensor_tempC == 85.0)
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
  outputDevices.intent.displayContent.status = heaterStatus;

  // Im Dialog wird die pendingDefaultTempC angezeigt
  if (heaterStatus.uiState == UiState::default_temp_dialog) {
    outputDevices.intent.displayContent.runtimeConfigData.default_tempC.value =
        pendingDefaultTempC_;
  } else {
    outputDevices.intent.displayContent.runtimeConfigData.default_tempC.value =
        runtimeConfig.get_default_tempC();
  }
}
//}}}

void SystemController::clampTempToConfigVals(float &target) {
  //{{{
  if (target > Config::kTempMaxC)
    target = Config::kTempMaxC;
  else if (target < Config::kTempMinC)
    target = Config::kTempMinC;
};
//}}}

void SystemController::cyclePages() {
  //{{{
  using LCDIntent = UiState;

  if (outputDevices.intent.lcd_cycleDirection ==
      OutputDevicesIntent::LcdCycleDirection::Right) {
    switch (heaterStatus.uiState) {
    case LCDIntent::Off:
      return;
    case LCDIntent::start_page:
      heaterStatus.uiState = LCDIntent::menu_page;
      break;

    case LCDIntent::menu_page:
      heaterStatus.uiState = LCDIntent::start_page;
      break;

      // Die folgenden Seiten sind Dialoge, die nicht regulär mit
      // durchcyclen erreicht werden können.
    case LCDIntent::default_temp_dialog:
      break;

    case LCDIntent::default_temp_menu:
      break;

    case LCDIntent::short_relais_impulse_menu:
      break;

    case LCDIntent::pulse_dialog:
      break;
    }
    return;
  }
  if (outputDevices.intent.lcd_cycleDirection ==
      OutputDevicesIntent::LcdCycleDirection::Left) {
    switch (heaterStatus.uiState) {
    case LCDIntent::Off:
      return;
    case LCDIntent::start_page:
      heaterStatus.uiState = LCDIntent::menu_page;
      break;
    case LCDIntent::menu_page:
      heaterStatus.uiState = LCDIntent::start_page;
      break;

      // Die folgenden Seiten sind Dialoge, die nicht regulär mit
      // durchcyclen erreicht werden können.
    case LCDIntent::default_temp_dialog:
      break;

    case LCDIntent::default_temp_menu:
      break;

    case LCDIntent::short_relais_impulse_menu:
      break;

    case LCDIntent::pulse_dialog:
      break;
    }
    return;
  }
}
//}}}

inline bool SystemController::displayButtonAction() {
  return inputDevices.data.modifier.released && !modifier_used_;
}

void SystemController::cycleMenuPages() {}

void SystemController::requestRelaisCommand(
    OutputDevicesIntent::RelaisCommand command) {
  outputDevices.intent.relaisCommand = command;
}
//}}}
