#include "runtime_config.h"
#include "config.h"
#include <EEPROM.h>

void RuntimeConfig::load() {
  EEPROM.get(default_tempC_addr, data.default_tempC);
  if (data.default_tempC <= 5 || data.default_tempC >= 30) {
    data.default_tempC = Config::kDefaultTempC;
  }

}

void RuntimeConfig::set_default_tempC(float new_default) {
  if (new_default == data.default_tempC) {
    return;
  }

  EEPROM.put(default_tempC_addr, new_default);
  data.default_tempC = new_default; 
}

float RuntimeConfig::get_default_tempC() const{
	return data.default_tempC;
}
