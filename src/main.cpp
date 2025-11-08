#include <Arduino.h>
#include "HCSR04.h"

// pin defines
const uint8_t TRIG_PIN = 12;
const uint8_t ECHO_PIN = 2;

HCSR04 sensor(TRIG_PIN, ECHO_PIN, 30000);  // 30 ms timeout ≈ ~5 m range

void setup() {
  Serial.begin(115200);
  sensor.begin();
}

void loop() {
  float distance_cm = sensor.read_cm();
  if (distance_cm < 0) {
    Serial.println("Out of range");
  } else {
    Serial.print("Distance: ");
    Serial.print(distance_cm);
    Serial.println(" cm");
  }
}