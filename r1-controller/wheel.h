#pragma once

#include <Servo.h>
#include <math.h>
#include "i2c.h"

float fmod_pos(float x, float m) {
  float r = fmod(x, m);
  return r < 0 ? r + m : r;
}
float angle_diff(float a, float b) {
  return fmod_pos(b - a + PI, 2 * PI) - PI;
}

namespace Wheel {
  constexpr int MINIMUM_MS = 1000;
  constexpr int MAXIMUM_MS = 2000;

  constexpr int ESC_PINS[I2C::COUNT] = {32, 33, 25, 26};
  constexpr int COUNTS_PER_REV[I2C::COUNT] = {4000, 4000, 4000, 4000};

  Servo escs[I2C::COUNT];
  float servoRadians[I2C::COUNT];

  void setup() {
    for (int i = 0; i < I2C::COUNT; i++) {
      escs[i].attach(ESC_PINS[i], MINIMUM_MS, MAXIMUM_MS);
      escs[i].writeMicroseconds(MINIMUM_MS);
    }
  }
  void sync() {
    for (int i = 0; i < I2C::COUNT; i++) {
        int counter = I2C::servoPositions[i];
        const int CPR = COUNTS_PER_REV[i];

        // Convert target angle to nearest count (within one revolution)
        int targetCount = (int)roundf(servoRadians[i] / (2.0f * PI) * CPR);

        // Integer-space wrap: find nearest multiple of cpr to bridge counter → targetCount
        int diff = targetCount - counter;
        diff = ((diff % CPR) + CPR) % CPR;  // always-positive modulo (int version)
        if (diff > CPR / 2) diff -= CPR;    // pick shorter direction

        I2C::servoPositions[i] = counter + diff;
    }
    I2C::sync();
  }
}
