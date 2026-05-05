#pragma once
#include "stdint.h"
#include "types.h"

class IDriver {
public:
  virtual void init() = 0;

protected: 
  ~IDriver() = default; 
};

class IInputDevices {
public:
  virtual void init() = 0;
  virtual void updateInputData() = 0;

protected:
  ~IInputDevices() = default;
};

class IOutputDevices {
public:
  virtual void init() = 0;
  virtual void update() = 0;

protected:
  ~IOutputDevices() = default;
};

class IDisplay {
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
  ~IDisplay() = default;
};

class IEncoder {
public:
  virtual int read() = 0;

protected:
  ~IEncoder() = default;
};
