#include "encoder_driver.h"
#

#ifdef TEST_BUILD 
#include "ArduinoStubs.h"
#else 
#include "Arduino.h"
#endif 

EncoderDriver::EncoderDriver(IEncoderHardware &encoderHardware)
    : m_encoderHardware(encoderHardware) {}

void EncoderDriver::init() {
  m_prev = m_encoderHardware.read(); 
  m_current = m_prev; 
  m_delta = 0; 
  m_last_change_ms = millis();  
};

int EncoderDriver::readSteps() {
  poll();
  return translatePositionToInput();
}

void EncoderDriver::poll() {
  m_current = m_encoderHardware.read();
  long diff = m_current - m_prev;
  if (diff == 0)
    return;
  m_delta += diff;
  m_last_change_ms = millis();
  m_prev = m_current;
}

int EncoderDriver::translatePositionToInput() {
  if (millis() - m_last_change_ms < DebounceConfig::kEncoderMs)
    return 0;
  int steps = m_delta / 4; // Wird auf ganze Schritte runtergebrochen und rest
                           // vernichtet bei Ganzzahldivision
  m_delta -= steps * 4;    // Das Delta wird um die Schritte die ganz gemacht
                           // wurden wieder reduziert (in Rohwerten) so bleibt
                           // der Rest für den nächsten Aufruf erhalten
  return steps;
};

