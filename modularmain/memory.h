#include "statistics.h" // Erwartet: struct LongtimeData { uint8_t dutyCycle; uint8_t avgIdleTime; }
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
      m_blockIndex++;

      if (m_blockIndex >= kBlockCount) {
        mergeBlocksToBaseline();
        m_blockIndex = 0;
      }
    }
  }

  LongtimeData getFinalAverages() const {
    LongtimeData result;

    int baselineWeight = EEPROM.read(kBaselineWeightAddr);
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
  static constexpr int kDCAvgAddr = 10;      // Laufender DC-Mittelwert
  static constexpr int kITAvgAddr = 11;      // Laufender IdleTime-Mittelwert
  static constexpr int kDCBaselineAddr = 12; // Baseline DC
  static constexpr int kITBaselineAddr = 13; // Baseline IT
  static constexpr int kBaselineWeightAddr = 14; // Gewicht der Baseline

  static constexpr int kDCBlockBaseAddr = 20; // DC-Blöcke starten hier
  static constexpr int kITBlockBaseAddr = 40; // IT-Blöcke starten hier

  // === Konfiguration ===
  static constexpr int kBlockCount = 10;
  static constexpr int kUpdateThreshold = 255; // Anzahl Updates pro Block
  static constexpr int kBlockWeight = 255;     // Gewicht eines Blocks
  static constexpr int kBaselineBlockWeight = kBlockCount * kBlockWeight;

  int m_writeCounter = 0;
  int m_blockIndex = 0;

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

  void commitToBlock() {
    int dcValue = EEPROM.read(kDCAvgAddr);
    int itValue = EEPROM.read(kITAvgAddr);

    int dcAddr = kDCBlockBaseAddr + m_blockIndex;
    int itAddr = kITBlockBaseAddr + m_blockIndex;

    EEPROM.write(dcAddr, dcValue);
    EEPROM.write(itAddr, itValue);
  }

  void mergeBlocksToBaseline() {
    int sumDC = 0;
    int sumIT = 0;

    for (int i = 0; i < kBlockCount; i++) {
      sumDC += EEPROM.read(kDCBlockBaseAddr + i);
      sumIT += EEPROM.read(kITBlockBaseAddr + i);
    }

    int blockAvgDC = sumDC / kBlockCount;
    int blockAvgIT = sumIT / kBlockCount;

    // Vorherige Baseline lesen
    int oldBaselineDC = EEPROM.read(kDCBaselineAddr);
    int oldBaselineIT = EEPROM.read(kITBaselineAddr);
    int oldWeight = EEPROM.read(kBaselineWeightAddr);

    // Neue Baseline berechnen mit Gewichtung
    int newBaselineDC =
        (oldBaselineDC * oldWeight + blockAvgDC * kBaselineBlockWeight) /
        (oldWeight + kBaselineBlockWeight);
    int newBaselineIT =
        (oldBaselineIT * oldWeight + blockAvgIT * kBaselineBlockWeight) /
        (oldWeight + kBaselineBlockWeight);

    // Schreiben
    EEPROM.write(kDCBaselineAddr, newBaselineDC);
    EEPROM.write(kITBaselineAddr, newBaselineIT);
    EEPROM.write(kBaselineWeightAddr, oldWeight + kBaselineBlockWeight);
  }
};
