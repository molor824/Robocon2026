#pragma once

#include <Wire.h>

#include "constants.h"

namespace I2C {
  constexpr int PIN_SCL = 22;
  constexpr int PIN_SDA = 21;
  constexpr int FREQ = 400000;

  constexpr int SERVO_ADDR = 69;

  // -180 to 180
  int servoPositions[Constant::WHEEL_COUNT] = {};

  void setup() {
    Wire.begin(PIN_SDA, PIN_SCL, FREQ);
  }

  void sync() {
    Wire.beginTransmission(SERVO_ADDR);
    Wire.write((uint8_t *)servoPositions, sizeof(servoPositions));
    Wire.endTransmission();
  }
}