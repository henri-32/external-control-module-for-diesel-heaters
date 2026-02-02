#pragma once
#include "Arduino.h"
#include "memory.h"
#include "types.h"
#include <EEPROM.h>
#include <avr/eeprom.h>
#include <limits.h>

struct RuntimeData {
  unsigned int dutyCycle = 0;
  unsigned long avgIdleTime_minutes;
  unsigned long maxIdleTime_minutes;
  unsigned int minIdleTime_minutes = UINT_MAX;
  unsigned int cycleCounter = 0;
  float mediumDiffTempToTarget = 0.0;
};

struct CalculationData {
  uint16_t updateCounter = 0;
  float TempDiffcontainer;
  unsigned long lastON = 0;
  unsigned long lastOFF = 0;
  unsigned long accumulatedTimeOFF = 0;
  unsigned long accumulatedTimeON = 0;
  unsigned long lastOFFperiodeLength = 0;
};

class SystemStatistics {
public:
  SystemStatistics() = default;

  void update(ControllerInputData &input, HeaterStatus &status) {
    timeStamp = millis();
    m_status = status;
    m_input = input;
    calculationData.updateCounter += 1;
    calculateDataValues();
    //writeLongTimeStats();
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
    calculateCycleCounter();
    calculateDiffTempToTarget();
    calculateDutyStats();
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
    m_lastState = m_status.heatingState;
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
    if (calculationData.lastOFFperiodeLength >
        runtimeData.maxIdleTime_minutes) {
      runtimeData.maxIdleTime_minutes =
          millisecondsToMinutes(calculationData.lastOFFperiodeLength);
    }
  }

  void calculateMinTimeBetweenDuty() {
    if (calculationData.lastOFFperiodeLength <
        runtimeData.minIdleTime_minutes) {
      runtimeData.minIdleTime_minutes =
          millisecondsToMinutes(calculationData.lastOFFperiodeLength);
    }
  }

  void calculateDutyCycle() {
    runtimeData.dutyCycle = (calculationData.accumulatedTimeON /
                             max(1, (calculationData.accumulatedTimeOFF +
                                     calculationData.accumulatedTimeON))) *
                            100;
  }

  int millisecondsToMinutes(unsigned long millisecs) {
    return millisecs / 1000 / 60;
  }
/*
  void writeLongTimeStats() {
    if (timeStamp - lastWrite < writingIntervall)
      return;

    uint8_t longTimeDutyCycle = EEPROM.read(memoryAdress.dutyCycle);
    uint8_t longTimeavgIdleTime_minutes = EEPROM.read(memoryAdress.avgIdleTime);
    uint8_t writeCycles = EEPROM.read(memoryAdress.writeCycles);

    EEPROM.write(memoryAdress.dutyCycle,
                 (longTimeDutyCycle += runtimeData.dutyCycle) /
                     max(1, writeCycles));
    EEPROM.write(memoryAdress.avgIdleTime, (longTimeavgIdleTime_minutes +=
                                            runtimeData.avgIdleTime_minutes) /
                                               max(1, writeCycles));

    lastWrite = timeStamp;
    writeCycles += 1;
    EEPROM.write(memoryAdress.writeCycles, writeCycles);
  }*/
};
