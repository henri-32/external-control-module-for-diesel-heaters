#include "interfaces.h"

class RuntimeConfig : public IRuntimeConfig {
public:
  RuntimeConfig() = default;
  void load();
  void set_default_tempC(float new_default);
  float get_default_tempC() const;

private:
  int default_tempC_addr = 0;
};
