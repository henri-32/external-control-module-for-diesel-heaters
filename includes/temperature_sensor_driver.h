#pragma once 
#include "interfaces.h"

class TemperatureSensorDriver : public ITempSensorDriver {
public:
  explicit TemperatureSensorDriver(ITempSensorHardware &sensorHardware);

  void init() override;
  float pollTemp() override;

private:
  void startTemperatureRequest();
  void measureTemperature();

  ITempSensorHardware &m_sensorHardware;

  unsigned long m_last_temp_request = 0;
  bool m_tempRequestPending = false;
  static constexpr unsigned long m_request_intervall_ms = 2000;
  static constexpr unsigned long m_conversion_time_ms = 750;
  float m_temp_c;
};
