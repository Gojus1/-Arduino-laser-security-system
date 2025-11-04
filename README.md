# Arduino-laser-security-system

Laser security system, that uses components like diode, servo motor, buzzer, zs-040 bluetooth connector, ldr's, laser, resistors.

Project PDF: 
(To be continued)

![Preview](IMG_2227.jpeg)

# Wiring Guide

Servo Motor:
  Signal (Orange) → Pin 9
  Power (Red) → 5V
  Ground (Brown) → GND

Laser Module:
  VCC → 5V
  GND → GND
  (Signal optional on Pin 7)

Photoresistor Array (3 zones):
  LDR_Left: 5V → LDR → A0 → 220Ω → GND
  LDR_Center: 5V → LDR → A1 → 220Ω → GND  
  LDR_Right: 5V → LDR → A2 → 220Ω → GND

Buzzer:
  Positive → Pin 8
  Negative → GND
