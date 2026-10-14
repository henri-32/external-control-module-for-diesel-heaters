#include "interfaces.h"

class RuntimeConfig : public IRuntimeConfig {
public:
  RuntimeConfig() = default;
  void load() override;
  void set_default_tempC(float value) override;
  float get_default_tempC() const override;

  void set_relais_pulse_short_ms(uint8_t value) override;
  uint8_t get_relais_pulse_short_ms() const override;

private:
  RuntimeConfigData data;
};
