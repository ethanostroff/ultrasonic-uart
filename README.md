# ultrasonic-uart

This project uses an Arduino Uno connected to an ultrasonic distance sensor (HC-SR04).
This README explains the physical wiring used for the setup described by the repository.

## Quick wiring summary

- Sensor VCC  -> Arduino 5V (red wire)
- Sensor Trig -> Arduino D12  (blue wire)
- Sensor Echo -> Arduino D2   (green wire)
- Sensor GND  -> Arduino GND (black wire)

## Toggle a light over UART

This project can also be used to toggle something (I am using a light) by sending a message over UART when a hand wave is detected by the ultrasonic sensor.