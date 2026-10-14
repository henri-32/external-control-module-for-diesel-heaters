#include "runtime_config.h"
#include "config.h"
#include <EEPROM.h>
#include <math.h>

namespace {

template <typename T>
void store_config_value(T value, ConfigMember<T> &config) {
  if (value == config.value) {
    return;
  }

  EEPROM.put(config.addr, value);
  config.value = value;
}

} // namespace

void RuntimeConfig::load() {
  EEPROM.get(data.default_tempC.addr, data.default_tempC.value);
  if (!isfinite(data.default_tempC.value)) {
    data.default_tempC.value = Config::kDefaultTempC;
  }

  EEPROM.get(data.RelaisPulseShortMs.addr, data.RelaisPulseShortMs.value);
  if (!isfinite(data.RelaisPulseShortMs.value)) {
    data.RelaisPulseShortMs.value = Config::kRelaisShortPulseMs;
  }
}

void RuntimeConfig::set_default_tempC(float value) {
  store_config_value(value, data.default_tempC);
}

float RuntimeConfig::get_default_tempC() const {
  return data.default_tempC.value;
}

void RuntimeConfig::set_relais_pulse_short_ms(uint8_t value) {
  store_config_value(value, data.RelaisPulseShortMs);
}

uint8_t RuntimeConfig::get_relais_pulse_short_ms() const {
  return data.RelaisPulseShortMs.value;
}
