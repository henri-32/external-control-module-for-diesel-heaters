#pragma once 
#include "interfaces.h"
#include "types.h"
#include <DallasTemperature.h>
#include <Encoder.h>
#include <OneWire.h>



#ifdef TEST_BUILD 
#include "ArduinoStubs.h"
#else 
#include "Arduino.h"
#endif 

class TemperatureSensorDriver : public IDriver {
public:
  explicit TemperatureSensorDriver(uint8_t pin);

  void init() override;
  float pollTemp();

private:
  void startTemperatureRequest();
  void measureTemperature();

  OneWire m_one_wire; //In Library findet Pin Zuweisung statt
  DallasTemperature m_sensors;

  unsigned long m_last_temp_request = 0;
  bool m_tempRequestPending = false;
  static constexpr unsigned long m_request_intervall_ms = 2000;
  static constexpr unsigned long m_conversion_time_ms = 750;
  float m_temp_c;
};
