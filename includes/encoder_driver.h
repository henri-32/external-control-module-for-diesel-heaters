#pragma once 
#include "interfaces.h"

class EncoderDriver : public IEncoderDriver {
//Treiberklasse für den Encoder
public:
  explicit EncoderDriver(IEncoderHardware &encoderHardware);

  void init() override; 
  int readSteps() override;

private:
  void poll();
  int translatePositionToInput();

  long m_current = 0;
  long m_prev = 0;
  long m_delta = 0;
  unsigned long m_last_change_ms = 0;
  IEncoderHardware &m_encoderHardware;
};
