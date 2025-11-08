#include "HCSR04.h"

// Constructor for an HCSR04 sensor
HCSR04::HCSR04(uint8_t trigPin, uint8_t echoPin, unsigned long timeout)
  : _trigPin(trigPin), _echoPin(echoPin), _timeout(timeout), _lastDuration(0) {}

// Initialize the sensor pins as an input and output
void HCSR04::begin() {
  pinMode(_trigPin, OUTPUT);
  digitalWrite(_trigPin, LOW);
  pinMode(_echoPin, INPUT);
}

// Sensor read configuration setter functions
void HCSR04::setTimeout(unsigned long timeout_us) { _timeout = timeout_us; }
void HCSR04::setSamples(uint8_t n) { _samples = (n < 3) ? 3 : (n | 1); } // force odd ≥3
void HCSR04::setGuardMs(uint16_t ms) { _guardMs = ms; }
void HCSR04::setAmbientTempC(float tC) { _ambientC = tC; }

// Get the duration of the last successful ping (or 0 on timeout)
unsigned long HCSR04::lastDuration() const { return _lastDuration; }

// Insertion sort for small arrays for the median filter
static void isort(float* a, uint8_t n) {
  for (uint8_t i = 1; i < n; ++i) {
    float key = a[i];
    int8_t j = i - 1;
    while (j >= 0 && a[j] > key) { a[j+1] = a[j]; j--; }
    a[j+1] = key;
  }
}

// Perform a single ping measurement and return echo duration in us or 0 on timeout
unsigned long HCSR04::pingOnce() {
  // 10 us trigger pulse
  digitalWrite(_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(_trigPin, LOW);

  // Prefer pulseInLong (better handling for long timeouts) or fall back if not present
  #if defined(ARDUINO_ARCH_AVR) || defined(pulseInLong)
    unsigned long duration = pulseInLong(_echoPin, HIGH, _timeout);
  #else
    unsigned long duration = pulseIn(_echoPin, HIGH, _timeout);
  #endif

  _lastDuration = duration;
  return duration; // 0 on timeout
}

// Convert echo time (us) to distance (cm) given ambient temperature celsius
float HCSR04::usToCm(unsigned long us, float tempC) {
  // Speed of sound (m/s) ~= 331.3 + 0.606 * T (celsius)
  const float c = 331.3f + 0.606f * tempC; // m/s

  // Round-trip time (us) for 1 cm at speed c:
  // distance (cm) = (us * c) / (2 * 1e6) * 100 = us * c / 20000
  return (us * c) / 20000.0f;
}

// Read distance in centimeters, using median of N samples
float HCSR04::read_cm() { return read_cm(_ambientC); }

// Read distance in centimeters, using median of N samples, with temperature override
float HCSR04::read_cm(float tempC) {
  // Guard time between measurement batches
  unsigned long nowMs = millis();
  if ((nowMs - _lastBatchMs) < _guardMs) {
    delay(_guardMs - (nowMs - _lastBatchMs));
  }
  _lastBatchMs = millis();

  // Collect N samples (skipping timeouts) with a small delay to let echoes fade
  const uint8_t N = _samples;
  float buf[15]; // supports up to 15 samples but can be adjusted for more accuracy if needed
  uint8_t got = 0;
  unsigned long startAll = micros();

  // Ping until we have N valid samples or timeout
  while (got < N) {
    unsigned long dur = pingOnce();
    if (dur > 0) {
      buf[got++] = usToCm(dur, tempC);
    }
    delay(12); // 12 ms between pings inside the batch
    
    // if too much time has passed, abort to avoid blocking
    if ((micros() - startAll) > (unsigned long)(_timeout + 40000)) break;
  }

  if (got == 0) return -1.0f; // no valid echoes

  // Median filter using insertion sort
  isort(buf, got);
  float cm = buf[got / 2];

  // clamp values since HC-SR04 spec ~2–400 cm usable
  if (cm < 2.0f)  cm = 2.0f;
  if (cm > 400.0f) cm = 400.0f;

  return cm;
}