#pragma once
#include "Arduino.h"
#include "types.h"
#include <EEPROM.h>
#include <avr/eeprom.h>


class SystemStatistics {
public:
  SystemStatistics() = default;

  void update(InputDevicesDataSet &inputData, HeaterStatus &status) {
    timeStamp = millis();
    m_status = status;
    data = inputData;
    calculationData.updateCounter += 1;
    calculateDataValues();
    writeLongTimeStats();
  }

  RuntimeData getRuntimeDate() const {
    return runtimeData;

  }

  LongtimeData getLongTimeData() const { 
    return longTimeDataBuffer;
  }

  static constexpr unsigned long writingIntervall = 2UL * 60UL * 60UL * 1000UL; // 2h
  unsigned long lastWrite = 0;
  unsigned long timeStamp;

private:
  RuntimeData runtimeData;
  LongtimeData longTimeDataBuffer;
  InputDevicesDataSet data;
  HeaterStatus m_status;
  HeaterStatus::State m_lastState = HeaterStatus::State::OFF;
  CalculationData calculationData;
  bool m_longTimeDataReady = false;

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
    if (m_status.heatingState == HeaterStatus::State::ON) {
      calculationData.lastON = timeStamp;
    } else if (m_status.heatingState == HeaterStatus::State::OFF) {
      calculationData.lastOFF = timeStamp;
    }
  }

  void calculateCycleCounter() {
    if (m_status.heatingState == m_lastState)
      return;
    if (m_status.heatingState == HeaterStatus::State::ON) {
      runtimeData.cycleCounter += 1;
    }
  }

  void calculateDiffTempToTarget() {
    calculationData.TempDiffcontainer +=
        data.sensor_tempC - m_status.target_tempC;
    runtimeData.mediumDiffTempToTarget =
        calculationData.TempDiffcontainer / calculationData.updateCounter;
  }

  void calculateDutyStats() {
    if (m_status.heatingState == m_lastState)
      return;
    if (m_status.heatingState == HeaterStatus::State::ON) {
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
    m_longTimeDataReady = true;
  };

public:
  bool takeLongTimeData(LongtimeData &out) {
    if (!m_longTimeDataReady)
      return false;
    out = longTimeDataBuffer;
    m_longTimeDataReady = false;
    return true;
  }
};

