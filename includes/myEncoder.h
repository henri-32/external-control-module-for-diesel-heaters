#pragma once 
#include "interfaces.h"

// Name gewählt, da Bibliothek Encoder heißt.
class MyEncoder : public IDriver {
public:
  explicit MyEncoder(IEncoder& encoder);

  void init() override { poll(); }
  int readSteps();

private:
  void poll();
  int translateStepsToInput();

  long m_current = 0;
  long m_prev = 0;
  long m_delta = 0;
  unsigned long m_last_change_ms = 0;
  IEncoder& m_encoder;
};

