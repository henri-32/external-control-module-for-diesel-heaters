// Experimentelle Features, die statistische Auswertung während der Laufzeit 
// ermöglichen sollten. Ich habe sie geschrieben, bevor ich wusste was Unit-Tests 
// sind. Dementsprechend ist irgendwo ein Bug drin und die Features sind noch nicht 
// zu gebrauchen 

#pragma once
#include "types.h"
#include <EEPROM.h>

class StatisticMemoryController {
public:
  StatisticMemoryController() = default;

  void update(const LongtimeData &data) {
    updateRunningAverage(data);
    m_writeCounter++;

    if (m_writeCounter >= kUpdateThreshold) {
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
    int baselineDC = readUint16(kDCBaselineAddr);
    int baselineIT = readUint16(kITBaselineAddr);

    int currentDC = readUint16(kDCAvgAddr);
    int currentIT = readUint16(kITAvgAddr);

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
  static constexpr int kDCAvgAddr = 10; // Laufender Mittelwert (uint16_t)
  static constexpr int kITAvgAddr = 12;

  static constexpr int kDCBaselineAddr = 14; // Langzeitwert (uint16_t)
  static constexpr int kITBaselineAddr = 16;

  static constexpr int kBaselineUpdateCountAddr = 18; // Zähler statt Gewicht

  static constexpr int kDCBlockBaseAddr = 20; // Block 0–9 (uint16_t)
  static constexpr int kITBlockBaseAddr = 40;

  // === Konstante Gewichtungskonfiguration ===
  static constexpr int kUpdateThreshold = 255; // Updates pro Block
  static constexpr int kBlockMaxCount = 10;
  static constexpr int kBlockWeight = 255;     // Gewicht eines Blocks
  static constexpr int kWeightPerBaseline = kBlockMaxCount * kBlockWeight;

  // === Zustand (nur im RAM, nicht persistent) ===
  int m_writeCounter = 0;
  int m_blockIndexOffset = 0;

  // === EEPROM-Mittelwert aktualisieren ===
  void updateRunningAverage(const LongtimeData &data) {
    uint16_t prevDC = readUint16(kDCAvgAddr);
    uint16_t prevIT = readUint16(kITAvgAddr);

    uint16_t newDC =
        (prevDC * kBlockWeight + data.dutyCycle) / (kBlockWeight + 1);
    uint16_t newIT =
        (prevIT * kBlockWeight + data.avgIdleTime) / (kBlockWeight + 1);

    writeUint16(kDCAvgAddr, newDC);
    writeUint16(kITAvgAddr, newIT);
  }

  // === Mittelwert in Block sichern ===
  void commitToBlock() {
    int dcValue = readUint16(kDCAvgAddr);
    int itValue = readUint16(kITAvgAddr);

    writeUint16(kDCBlockBaseAddr + (m_blockIndexOffset * 2), dcValue);
    writeUint16(kITBlockBaseAddr + (m_blockIndexOffset * 2), itValue);
  }

  // === Blöcke zu Baseline zusammenführen ===
  void mergeBlocksToBaseline() {
    int sumDC = 0;
    int sumIT = 0;

    for (int i = 0; i < kBlockMaxCount; i++) {
      sumDC += readUint16(kDCBlockBaseAddr + (i * 2));
      sumIT += readUint16(kITBlockBaseAddr + (i * 2));
    }

    int blockAvgDC = sumDC / kBlockMaxCount;
    int blockAvgIT = sumIT / kBlockMaxCount;

    int oldBaselineDC = readUint16(kDCBaselineAddr);
    int oldBaselineIT = readUint16(kITBaselineAddr);
    int oldWeight = readBaselineWeight();

    int newBaselineDC =
        (oldBaselineDC * oldWeight + blockAvgDC * kWeightPerBaseline) /
        (oldWeight + kWeightPerBaseline);
    int newBaselineIT =
        (oldBaselineIT * oldWeight + blockAvgIT * kWeightPerBaseline) /
        (oldWeight + kWeightPerBaseline);

    writeUint16(kDCBaselineAddr, newBaselineDC);
    writeUint16(kITBaselineAddr, newBaselineIT);
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

  uint16_t readUint16(int addr) const {
    uint16_t value = 0;
    EEPROM.get(addr, value);
    return value;
  }

  void writeUint16(int addr, uint16_t value) {
    EEPROM.put(addr, value);
  }
};

