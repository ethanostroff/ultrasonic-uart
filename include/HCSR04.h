#include <Arduino.h>

class HCSR04 {
public:
  HCSR04(uint8_t trigPin, uint8_t echoPin, unsigned long timeout_us = 30000);

  void begin();

  void setTimeout(unsigned long timeout_us);
  void setSamples(uint8_t n); // how many readings per measurement (median)
  void setGuardMs(uint16_t ms); // minimum time between measurement batches
  void setAmbientTempC(float tC); // speed-of-sound compensation

  unsigned long lastDuration() const;
  float read_cm(); // uses configured ambient temp
  float read_cm(float tempC); // one-off temp override

private:
  uint8_t  _trigPin, _echoPin;
  unsigned long _timeout;
  unsigned long _lastDuration;
  uint8_t  _samples = 7; // number for median
  uint16_t _guardMs = 60;
  float    _ambientC = 20.0f; // default 20 celcius
  unsigned long _lastBatchMs = 0;

  unsigned long pingOnce(); // returns echo duration (us) or 0 on timeout
  static float  usToCm(unsigned long us, float tempC);
};