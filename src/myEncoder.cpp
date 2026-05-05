#include "myEncoder.h"

#ifdef TEST_BUILD 
#include "ArduinoStubs.h"
#else 
#include "Arduino.h"
#endif 

MyEncoder::MyEncoder(IEncoder& encoder) : m_encoder(encoder){}

int MyEncoder::readSteps() {
  poll();
  return translateStepsToInput();
}

void MyEncoder::poll() {
  m_current = m_encoder.read();
  long diff = m_current - m_prev;
  if (diff == 0)
    return;
  m_delta += diff;
  m_last_change_ms = millis();
  m_prev = m_current;
}

int MyEncoder::translateStepsToInput() {
  if (millis() - m_last_change_ms < 100)
    return 0;
  int steps = m_delta / 4; // Wird auf ganze Schritte runtergebrochen und rest
                           // vernichtet bei Ganzzahldivision
  m_delta -= steps * 4;    // Das Delta wird um die Schritte die ganz gemacht
                           // wurden wieder reduziert (in Rohwerten) so bleibt
                           // der Rest für den nächsten Aufruf erhalten
  return steps;
};


