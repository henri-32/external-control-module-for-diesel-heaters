#include "interfaces.h"

class ModifiableConfig : public IModifiableConfig {
public:
  ModifiableConfig() = default;
  void load();
  void set_default_tempC(float new_default);
  float get_default_tempC() const;

private:
  int default_tempC_addr = 0;
};
