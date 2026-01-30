#pragma once

#include "types.h"

struct StatisticData {

  int dutyCycle;
  int mediumTimeBetweenDuty;
  int maxTimeBetweenDuty;
  int minTimeBetweenDuty;
  int cycleCount;
  float mediumDiffTempToTarget;
};

struct CalculationContainers {
    float TempDiff [2] = {0, 0}; 
};

class SystemStatistics {
public:
  SystemStatistics() = default;

  void update(ControllerInputData input, HeaterStatus status) {
    m_status = status;
    m_input = input;
  }

private:
  static constexpr unsigned long writingIntervall = 0;
  static const unsigned long lastWrite = 0;

  StatisticData statisticData;
  ControllerInputData m_input;
  HeaterStatus m_status;
  CalculationContainers calcutationContainer;

  void calculateData() { calculateCycleCount(); }

  void calculateCycleCount() {
    if (m_status.heatingState == m_lastState)
      return;
    if (m_status.heatingState == HeaterStatus::HeatingState::ON) {
      statisticData.cycleCount += 1;
    }
    m_lastState = m_status.heatingState;
  }

  HeaterStatus::HeatingState m_lastState = HeaterStatus::HeatingState::OFF;

  void calculateDiffTempToTarget (){
    calcutationContainer.TempDiff[0] += m_input.sensor_tempC; 
    calcutationContainer.TempDiff[1] += m_status.target_temp_c;

    statisticData.mediumDiffTempToTarget = calcutationContainer.TempDiff[0] - calcutationContainer.TempDiff[1];
  }
};