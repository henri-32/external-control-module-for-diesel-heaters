#pragma once
#include "Arduino.h"
#include "memory.h"
#include "types.h"
#include <EEPROM.h>
#include <avr/eeprom.h>


class SystemStatistics {
public:
  SystemStatistics() = default;

  void update(ControllerInputData &input, HeaterStatus &status) {
    timeStamp = millis();
    m_status = status;
    m_input = input;
    calculationData.updateCounter += 1;
    calculateDataValues();
    writeLongTimeStats();
  }

  RuntimeData getRuntimeDate()const {
    return runtimeData;
  }

private:
  static constexpr unsigned long writingIntervall = 120000; // 2h
  unsigned long lastWrite = 0;
  unsigned long timeStamp;

  RuntimeData runtimeData;
  LongtimeData longTimeDataBuffer;
  ControllerInputData m_input;
  HeaterStatus m_status;
  HeaterStatus::HeatingState m_lastState = HeaterStatus::HeatingState::OFF;
  CalculationData calculationData;
  StatisticMemoryController memoryController;

  void calculateDataValues() {
    rememberLastON_OFF();
    calculateDutyStats();
    calculateCycleCounter();
    calculateDiffTempToTarget();
    m_lastState = m_status.heatingState;
  }

  void rememberLastON_OFF() {
    if (m_status.heatingState == m_lastState)
      return;
    if (m_status.heatingState == HeaterStatus::HeatingState::ON) {
      calculationData.lastON = timeStamp;
    } else if (m_status.heatingState == HeaterStatus::HeatingState::OFF) {
      calculationData.lastOFF = timeStamp;
    }
  }

  void calculateCycleCounter() {
    if (m_status.heatingState == m_lastState)
      return;
    if (m_status.heatingState == HeaterStatus::HeatingState::ON) {
      runtimeData.cycleCounter += 1;
    }
  }

  void calculateDiffTempToTarget() {
    calculationData.TempDiffcontainer +=
        m_input.sensor_tempC - m_status.target_temp_c;
    runtimeData.mediumDiffTempToTarget =
        calculationData.TempDiffcontainer / calculationData.updateCounter;
  }

  void calculateDutyStats() {
    if (m_status.heatingState == m_lastState)
      return;
    if (m_status.heatingState == HeaterStatus::HeatingState::ON) {
      calculationData.accumulatedTimeOFF += timeStamp - calculationData.lastOFF;
    } else {
      calculationData.accumulatedTimeON += timeStamp - calculationData.lastON;
    }

    calculationData.lastOFFperiodeLength = timeStamp - calculationData.lastOFF;

    calculateMediumTimeBetweenDuty();
    calculateMaxTimeBetweenDuty();
    calculateMinTimeBetweenDuty();
    calculateDutyCycle();
  }

  void calculateMediumTimeBetweenDuty() {
    runtimeData.avgIdleTime_minutes = millisecondsToMinutes(
        calculationData.accumulatedTimeOFF / max(1, runtimeData.cycleCounter));
  }

  void calculateMaxTimeBetweenDuty() {
    unsigned long lastOffMinutes =
        millisecondsToMinutes(calculationData.lastOFFperiodeLength);
    if (lastOffMinutes > runtimeData.maxIdleTime_minutes) {
      runtimeData.maxIdleTime_minutes = lastOffMinutes;
    }
  }

  void calculateMinTimeBetweenDuty() {
    unsigned long lastOffMinutes =
        millisecondsToMinutes(calculationData.lastOFFperiodeLength);
    if (lastOffMinutes < runtimeData.minIdleTime_minutes) {
      runtimeData.minIdleTime_minutes = lastOffMinutes;
    }
  }

  void calculateDutyCycle() {
    unsigned long total =
        max(1UL, (calculationData.accumulatedTimeOFF +
                  calculationData.accumulatedTimeON));
    runtimeData.dutyCycle =
        static_cast<unsigned int>(
            (static_cast<float>(calculationData.accumulatedTimeON) * 100.0f) /
            static_cast<float>(total));
  }

  int millisecondsToMinutes(unsigned long millisecs) {
    return millisecs / 1000 / 60;
  }

  void writeLongTimeStats() {
    if (timeStamp - lastWrite < writingIntervall)
      return;
    lastWrite = timeStamp;

    longTimeDataBuffer.avgIdleTime = runtimeData.avgIdleTime_minutes;
    longTimeDataBuffer.dutyCycle = runtimeData.dutyCycle;

    memoryController.update(longTimeDataBuffer);
  };
};
