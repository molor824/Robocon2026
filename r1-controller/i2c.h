#pragma once

#include "Wire.h"

namespace I2C {
  constexpr int PIN_SCL = 22;
  constexpr int PIN_SDA = 21;
  constexpr int FREQ = 400000;

  constexpr int SERVO_ADDR = 69;
  constexpr int SERVO_COUNT = 4;

  float servoPositions[SERVO_COUNT] = {};

  void setup() {
    Wire.begin(PIN_SDA, PIN_SCL, FREQ);
  }

  void sync() {
    Wire.beginTransmission(SERVO_ADDR);
    Wire.write((uint8_t *)servoPositions, sizeof(servoPositions));
    Wire.endTransmission();
  }
}