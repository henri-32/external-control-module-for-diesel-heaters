#pragma once
#include "interfaces.h"

class TemperatureSensorDriver : public ITempSensorDriver {
//Treiberklasse für den Temperatursensor
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
  static constexpr unsigned long kRequestIntervalMs =
      Config::kTemperatureRequestIntervalMs;

  // Hardcoded, because it's no config value, but a DS18B20 Hardware
  // related timespan
  static constexpr unsigned long kConversionTimeMs = 750;
  float m_temp_c = Config::kDefaultTempC;
};
