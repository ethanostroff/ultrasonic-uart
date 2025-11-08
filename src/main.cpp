#include <Arduino.h>
#include "HCSR04.h"

// pin defines
const uint8_t TRIG_PIN = 12;
const uint8_t ECHO_PIN = 2;

bool lightsOn = false;

HCSR04 sensor(TRIG_PIN, ECHO_PIN, 30000);  // 30 ms timeout ≈ ~5 m range

// Forward declarations of debounced functions
bool debouncedNear(float thresh_cm, uint8_t need, uint16_t gap_ms = 0);
bool debouncedFar(float thresh_cm, uint8_t need, uint16_t gap_ms = 0);

void setup() {
  // start serial
  Serial.begin(115200);
  
  // start sensor
  sensor.begin();

  // configure sensor
  sensor.setSamples(7); // 5–11 is a good range
  sensor.setGuardMs(60); // ~60 ms between batches
  sensor.setAmbientTempC(22.0f); // room temp
  
  Serial.println("Ready: wave hand to toggle lights");
}

void loop() {
  // Simple latch (only allow a toggle once per “approach”, then re-arm after moving away)
  static bool armed = true; // true - waiting for near event
  const float NEAR = 35.0f; // cm (toggle threshold)
  const float REARM = 40.0f;  // cm (move away to re-arm)
  const uint8_t DEBOUNCE = 2; // consecutive confirmations
  const uint16_t SAMPLE_GAP_MS = 0; // keep 0, sensor already spaces pings

  if (armed) {
    // If we’re armed and we get a debounced “near”, toggle once
    if (debouncedNear(NEAR, DEBOUNCE, SAMPLE_GAP_MS)) {
      // update lights state
      lightsOn = !lightsOn;

      // transmit lights state over uart
      Serial.println(lightsOn ? "LIGHTS_ON" : "LIGHTS_OFF");
      armed = false; // disarm until hand moves away
    }
  } else {
    // Re-arm only after a debounced “far”
    if (debouncedFar(REARM, DEBOUNCE, SAMPLE_GAP_MS)) {
      armed = true;
    }
  }
}

// Returns true only if we get `need` consecutive valid reads <= thresh_cm.
bool debouncedNear(float thresh_cm, uint8_t need, uint16_t gap_ms = 0) {
  for (uint8_t i = 0; i < need; ++i) {
    float cm = sensor.read_cm();
    if (cm < 0 || cm > thresh_cm) return false;  // timeout or too far
    if (gap_ms) delay(gap_ms);
  }
  return true;
}

// Returns true only if we get `need` consecutive valid reads >= thresh_cm.
bool debouncedFar(float thresh_cm, uint8_t need, uint16_t gap_ms = 0) {
  for (uint8_t i = 0; i < need; ++i) {
    float cm = sensor.read_cm();
    if (cm < 0 || cm < thresh_cm) return false;  // timeout or still near
    if (gap_ms) delay(gap_ms);
  }
  return true;
}