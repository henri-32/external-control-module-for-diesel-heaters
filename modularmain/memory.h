#pragma once
#include "types.h"
#include <EEPROM.h>

class StatisticMemoryController {
public:
  StatisticMemoryController() = default;

  void update(const LongtimeData &data) {
    updateRunningAverage(data);
    m_writeCounter++;

    if (m_writeCounter >= kAddrCapacity) {
      commitToBlock();
      m_writeCounter = 0;
      m_blockIndexOffset++;

      if (m_blockIndexOffset >= kBlockMaxCount) {
        mergeBlocksToBaseline();
        m_blockIndexOffset = 0;
        incrementBaselineUpdateCount();
      }
    }
  }

  LongtimeData getFinalAverages() const {
    LongtimeData result;

    int baselineWeight = readBaselineWeight();
    int baselineDC = EEPROM.read(kDCBaselineAddr);
    int baselineIT = EEPROM.read(kITBaselineAddr);

    int currentDC = EEPROM.read(kDCAvgAddr);
    int currentIT = EEPROM.read(kITAvgAddr);

    // Kombiniere aktuelle Mittelwerte mit Baseline gewichtet
    result.dutyCycle =
        (baselineDC * baselineWeight + currentDC * kBlockWeight) /
        (baselineWeight + kBlockWeight);
    result.avgIdleTime =
        (baselineIT * baselineWeight + currentIT * kBlockWeight) /
        (baselineWeight + kBlockWeight);

    return result;
  }

private:
  // === EEPROM-Adressen ===
  static constexpr int kDCAvgAddr = 10; // Laufender Mittelwert
  static constexpr int kITAvgAddr = 11;

  static constexpr int kDCBaselineAddr = 12; // Langzeitwert
  static constexpr int kITBaselineAddr = 13;

  static constexpr int kBaselineUpdateCountAddr = 14; // Zähler statt Gewicht

  static constexpr int kDCBlockBaseAddr = 20; // Block 0–9 für DutyCycle
  static constexpr int kITBlockBaseAddr = 40;

  // === Konstante Gewichtungskonfiguration ===
  static constexpr int kAddrCapacity = 10; // Anzahl Blöcke bis Baseline
  static constexpr int kBlockMaxCount = 10;
  static constexpr int kUpdateThreshold = 255; // Updates pro Block
  static constexpr int kBlockWeight = 255;     // Gewicht eines Blocks
  static constexpr int kWeightPerBaseline = kBlockMaxCount * kBlockWeight;

  // === Zustand (nur im RAM, nicht persistent) ===
  int m_writeCounter = 0;
  int m_blockIndexOffset = 0;

  // === EEPROM-Mittelwert aktualisieren ===
  void updateRunningAverage(const LongtimeData &data) {
    uint8_t prevDC = EEPROM.read(kDCAvgAddr);
    uint8_t prevIT = EEPROM.read(kITAvgAddr);

    uint8_t newDC =
        (prevDC * kBlockWeight + data.dutyCycle) / (kBlockWeight + 1);
    uint8_t newIT =
        (prevIT * kBlockWeight + data.avgIdleTime) / (kBlockWeight + 1);

    EEPROM.write(kDCAvgAddr, newDC);
    EEPROM.write(kITAvgAddr, newIT);
  }

  // === Mittelwert in Block sichern ===
  void commitToBlock() {
    int dcValue = EEPROM.read(kDCAvgAddr);
    int itValue = EEPROM.read(kITAvgAddr);

    EEPROM.write(kDCBlockBaseAddr + m_blockIndexOffset, dcValue);
    EEPROM.write(kITBlockBaseAddr + m_blockIndexOffset, itValue);
  }

  // === Blöcke zu Baseline zusammenführen ===
  void mergeBlocksToBaseline() {
    int sumDC = 0;
    int sumIT = 0;

    for (int i = 0; i < kBlockMaxCount; i++) {
      sumDC += EEPROM.read(kDCBlockBaseAddr + i);
      sumIT += EEPROM.read(kITBlockBaseAddr + i);
    }

    int blockAvgDC = sumDC / kBlockMaxCount;
    int blockAvgIT = sumIT / kBlockMaxCount;

    int oldBaselineDC = EEPROM.read(kDCBaselineAddr);
    int oldBaselineIT = EEPROM.read(kITBaselineAddr);
    int oldWeight = readBaselineWeight();

    int newBaselineDC =
        (oldBaselineDC * oldWeight + blockAvgDC * kWeightPerBaseline) /
        (oldWeight + kWeightPerBaseline);
    int newBaselineIT =
        (oldBaselineIT * oldWeight + blockAvgIT * kWeightPerBaseline) /
        (oldWeight + kWeightPerBaseline);

    EEPROM.write(kDCBaselineAddr, newBaselineDC);
    EEPROM.write(kITBaselineAddr, newBaselineIT);
  }

  // === Baseline-Gewicht zur Laufzeit berechnen ===
  int readBaselineWeight() const {
    uint8_t updateCount = EEPROM.read(kBaselineUpdateCountAddr);
    return updateCount * kWeightPerBaseline;
  }

  void incrementBaselineUpdateCount() {
    uint8_t count = EEPROM.read(kBaselineUpdateCountAddr);
    if (count < 255) {
      EEPROM.write(kBaselineUpdateCountAddr, count + 1);
    }
  }
};
