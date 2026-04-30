#pragma once
#include "interfaces.h"
#include "types.h"
#include <Arduino.h>
#include <DallasTemperature.h>
#include <Encoder.h>
#include <OneWire.h>

class ToggleSwitchDriver {
public:
  explicit ToggleSwitchDriver(uint8_t pin);

  void init();
  bool changed();

private:
  const uint8_t m_pin;
  bool m_current;
  bool m_prev;
  unsigned long m_last_debounce_ms = 0;
  static constexpr unsigned long m_debounce_delay_ms = 50;
};

class PushButtonDriver {
public:
  explicit PushButtonDriver(uint8_t pin);

  void init();
  bool isDown() const { return m_stable == LOW; }
  bool released();

private:
  const uint8_t m_pin;
  bool m_stable;
  bool m_last_read;
  unsigned long m_last_debounce_ms = 0;
  static constexpr unsigned long m_debounce_delay_ms = 50;
};
// Name gewählt, da Bibliothek Encoder heißt.
class MyEncoderDriver {
public:
  explicit MyEncoderDriver(uint8_t pin1, uint8_t pin2);

  void init() { poll(); }
  int readSteps();

private:
  void poll();
  int translateStepsToInput();

  Encoder m_encoder; // Encoder library genutzt: Dort findet Pin Zuordnung statt
  long m_current = 0;
  long m_prev = 0;
  long m_delta = 0;
  unsigned long m_last_change_ms = 0;
};

class TemperatureSensorDriver {
public:
  explicit TemperatureSensorDriver(uint8_t pin);

  void init();
  float pollTemp();

private:
  void startTemperatureRequest();
  void measureTemperature();

  OneWire m_one_wire; // OneWire Lib genutzt: Dort findet Pin Zuweisung statt
  DallasTemperature m_sensors; // DallasTemperature Lib genutzt für
                               // Temperatursensor Timing
  unsigned long m_last_temp_request = 0;
  bool m_tempRequestPending = false;
  static constexpr unsigned long m_request_intervall_ms = 2000;
  static constexpr unsigned long m_conversion_time_ms = 750;
  float m_temp_c;
};

class RelaisDriver {
public:
  explicit RelaisDriver(const uint8_t pin);

  void init() ;
  void update(ControllerOutputIntent::RelaisCommand intent);

private:
  void
  applyPulseLengthFromIntent(ControllerOutputIntent::RelaisCommand intent_copy);
  void activate() { digitalWrite(m_pin, HIGH); }
  void deactivate() { digitalWrite(m_pin, LOW); };

  const uint8_t m_pin;
  enum class RelaisState { ON, OFF };
  RelaisState m_relais_state = RelaisState::OFF;
  uint16_t m_pulse_ms = 0;
  unsigned long m_pulse_start_ms = 0;
};
