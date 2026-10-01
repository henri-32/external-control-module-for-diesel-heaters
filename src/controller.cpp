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
    return;
  }

  // Gedrückter Modifier und Encoder wechseln die UIStates, vorausgesetzt
  // der Bildschirm ist angeschaltet
  if (id.modifier.pressed && !id.modifier.released && val != 0 &&
      state != State::Off) {
    if (val >= 1 && val <= Config::kEncoderValCutoff) {
      oi.lcd_cycleDirection = LCDDirection::Right;
      cyclePages();
      return;
    }
    if (val <= -1 && val >= -Config::kEncoderValCutoff) {
      oi.lcd_cycleDirection = LCDDirection::Left;
      cyclePages();
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

    // Drücken des Display Schalters schaltet das Display Aus
    DisplayButtonTurnsDisplayOff();

    // Der Encoder verstellt die Solltemperatur
    if (val == 0 || val >= Config::kEncoderValCutoff ||
        val <= -Config::kEncoderValCutoff) {
      return;
    }
    // val ist signed
    heaterStatus.target_tempC += val * Config::kTempStepC;
    clampTempToConfigVals(heaterStatus.target_tempC);

    break;

  case State::default_temp_page:
    // Die Seite für die default_temp bietet einen Dialog an, welcher durch
    // drücken des Modusschalters geöffnet wir (anderer UIState)
    if (id.switchAction.mode) {
      pendingDefaultTempC = runtimeConfig.get_default_tempC();
      state = State::default_temp_dialog;
      return;
    }

    // Ansonsten schaltet der DisplayButton das Display aus
    DisplayButtonTurnsDisplayOff();
    break;

    // Der Encoder wird bewusst nicht behandelt, da auf dieser Seite ohne
    // Funktion

  case State::default_temp_dialog:
    /*Im geöffneten default_temp Dialog wird durch den Encoder ein
     * pendingDefault Wert verstellt. Der Modusschalter übernimmt diesen, der
     * DisplayButton verwirft ihn und kehrt zum vorherigen default_temp UIState
     * zurück.
     */

    // Übernahme des pendingDefault Werts
    if (id.switchAction.mode) {
      runtimeConfig.set_default_tempC(pendingDefaultTempC);
      state = State::default_temp_page;
      return;
    }

    // Keine Übernahme
    if (id.modifier.released) {
      state = State::default_temp_page;
    }

    // Bei gültigen Encoderbefehlen pending Value anpassen
    if (val == 0 || val >= Config::kEncoderValCutoff ||
        val <= -Config::kEncoderValCutoff) {
      return;
    }
    pendingDefaultTempC += val * Config::kTempStepC;
    clampTempToConfigVals(pendingDefaultTempC);
    break;

  case State::Off:
    // Bei ausgeschaltetem Display hat nur der DisplayButton Funtion und
    // schaltet das Display ein.
    if (id.modifier.released && !id.modifier.pressed) {
      state = State::start_page;
    }
    break;
  }
};
//}}}

void SystemController::DisplayButtonTurnsDisplayOff() {
  //{{{
  if (inputDevices.data.modifier.released) {
    heaterStatus.uiState = UiState::Off;
    return;
  }
}
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
    outputDevices.intent.displayContent.runtimeConfigData.default_tempC =
        pendingDefaultTempC;
  } else {
    outputDevices.intent.displayContent.runtimeConfigData.default_tempC =
        runtimeConfig.get_default_tempC();
  }
}
//}}}

// =============Helper Functions
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
      heaterStatus.uiState = LCDIntent::default_temp_page;
      break;

    case LCDIntent::default_temp_page:
      heaterStatus.uiState = LCDIntent::start_page;
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
    switch (heaterStatus.uiState) {
    case LCDIntent::Off:
      return;
    case LCDIntent::start_page:
      heaterStatus.uiState = LCDIntent::default_temp_page;
      break;
    case LCDIntent::default_temp_page:
      heaterStatus.uiState = LCDIntent::start_page;
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
