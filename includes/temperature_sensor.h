#pragma once 
#include "interfaces.h"

class TemperatureSensor : public IDriver {
public:
  explicit TemperatureSensor(ITempSensor& sensor);

  void init() override;
  float pollTemp();

private:
  void startTemperatureRequest();
  void measureTemperature();

  ITempSensor& m_sensors;

  unsigned long m_last_temp_request = 0;
  bool m_tempRequestPending = false;
  static constexpr unsigned long m_request_intervall_ms = 2000;
  static constexpr unsigned long m_conversion_time_ms = 750;
  float m_temp_c;
};
