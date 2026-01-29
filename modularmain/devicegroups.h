#pragma once
#include "hardwaredrivers.h"
#include "types.h"
class InputDevices {
private:
  friend class SystemController;
  ToggleSwitchDriver m_powerSwitch{2};
  ToggleSwitchDriver m_modeSwitch{3};
  PushButtonDriver m_displayButton{8};
  MyEncoderDriver m_myEncoder{6, 7};
  TemperatureSensorDriver m_tempSensor{5};
  ControllerInputData &m_devices_data;

  InputDevices(ControllerInputData &id) : m_devices_data(id) {}

  void init() {
    m_powerSwitch.init();
    m_modeSwitch.init();
    m_displayButton.init();
    m_myEncoder.init();
    m_tempSensor.init();
  }

  void updateInputData() {
    m_devices_data.powerSwitchChanged = m_powerSwitch.changed();
    m_devices_data.modeSwitchChanged = m_modeSwitch.changed();
    m_devices_data.displayButtonChanged = m_displayButton.pressed();
    m_devices_data.encoder_val = m_myEncoder.readSteps();
    m_devices_data.sensor_tempC = m_tempSensor.pollTemp();
  }
};

class OutputDevices {
private:
  friend class SystemController;
  RelaisDriver m_relais{4};
  ControllerOutputIntent &m_devices_intent;
  DisplayDriver m_lcdDisplay; // Keine PIN Zuweisung nötig, da Arduino I2C Bus
                              // automatisch erkennt.
  OutputDevices(ControllerOutputIntent &oi)
      : m_devices_intent(oi), m_lcdDisplay(m_devices_intent.displayContent,
                                           m_devices_intent.lcd_stateIntent) {}

  void init() {
    m_relais.init();
    m_lcdDisplay.init();
  }

  void update() {
    m_relais.update(m_devices_intent.consumeRelaisRequest());
    m_lcdDisplay.update();
  }
};
