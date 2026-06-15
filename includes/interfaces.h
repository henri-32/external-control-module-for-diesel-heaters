#pragma once
#include "types.h"

//Verschiedene abstrakte Interface Klassen 
//Sie wurden nachträglich auf die Treiberklassen drauf gesetzt. 
//Daher bestanden alle Methoden schon in den derived classes. 
//Sie wurden ins Interface aufgenommen, um vollständige Implementierung 
//der TestStubs sicherzustellen 
 
class IInputDevices {
  //{{{
public:
  IInputDevices(InputDevicesDataSet &inputData) : data(inputData){};
  virtual void init() = 0;
  virtual void update() = 0;

  InputDevicesDataSet &data;

protected:
  ~IInputDevices() = default;
};
//}}}

class IOutputDevices {
  //{{{
public:
  IOutputDevices(OutputDevicesIntent &outputIntent) : intent(outputIntent){};
  virtual void init() = 0;
  virtual void update() = 0;

  OutputDevicesIntent &intent;

protected:
  ~IOutputDevices() = default;
};
//}}}

class IDriver {
  //{{{
public:
  virtual void init() = 0;

protected:
  ~IDriver() = default;
};
//}}}

class IDisplayDriver : public IDriver {
  //{{{
public:
  virtual void update() = 0;

protected:
  ~IDisplayDriver() = default;
};
//}}}

class IDisplayHardware {
  //{{{
public:
  virtual void clear() = 0;
  virtual void noDisplay() = 0;
  virtual void display() = 0;
  virtual void noBacklight() = 0;
  virtual void backlight() = 0;
  virtual void setCursor(uint8_t, uint8_t) = 0;
  virtual void printstr(const char[]) = 0;
  virtual void init() = 0;

protected:
  ~IDisplayHardware() = default;
};
//}}}

class IEncoderDriver : public IDriver {
  //{{{
public:
  virtual int readSteps() = 0;

protected:
  ~IEncoderDriver() = default;
};
//}}}

class IEncoderHardware {
  //{{{
public:
  virtual int read() = 0;

protected:
  ~IEncoderHardware() = default;
};
//}}}

class ITempSensorDriver : public IDriver {
  //{{{
public:
  virtual float pollTemp() = 0;

protected:
  ~ITempSensorDriver() = default;
};
//}}}

class ITempSensorHardware {
  //{{{
public:
  virtual void begin() = 0;
  virtual void requestTemperatures() = 0;
  virtual float getTempCByIndex(uint8_t) = 0;

protected:
  ~ITempSensorHardware() = default;
};
//}}}

class IToggleSwitch : public IDriver {
  //{{{
public:
  virtual bool changed() = 0;

protected:
  ~IToggleSwitch() = default;
};
//}}}

class IPushButton : public IDriver {
  //{{{
public:
  virtual bool pressed() const = 0;
  virtual bool released() = 0;

protected:
  ~IPushButton() = default;
};
//}}}

class IRelais : public IDriver {
  //{{{
public:
  virtual void update(OutputDevicesIntent::RelaisCommand intent) = 0;

protected:
  ~IRelais() = default;
};
//}}}

