#include "interfaces.h"
#include "types.h"


class TestInputDevices : public IInputDevices {
public:
  TestInputDevices(ControllerInputData &id);
  void init() override {}
  void updateInputData() override {}
};

class TestOutputDevices : public IOutputDevices {
public:
  TestOutputDevices(ControllerOutputIntent &oi);
  void init() override {}
  void update() override {}

};


