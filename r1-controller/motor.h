#pragma once

#include "spi.h"
#include "hal/ledc_types.h"
#include "esp32-hal-ledc.h"

namespace Motor {
  constexpr int COUNT = 4;
  constexpr int PWM_PINS[COUNT] = {14, 27, 17, 16};

  constexpr int INA_BITS[COUNT] = {6, 4, 0, 2};
  constexpr int INB_BITS[COUNT] = {7, 5, 1, 3};

  constexpr int DIRS[COUNT] = {1, 1, 1, 1};

  constexpr int FREQ = 20000;
  constexpr int RESOLUTION = 8;

  void setup() {
    for (int i = 0; i < COUNT; i++) {
      ledcSetup(i, FREQ, RESOLUTION);
      ledcAttachPin(PWM_PINS[i], i);
    }
  }
  void setSpeed(int index, int speed) {
    speed *= DIRS[index];
    if (speed > 255) speed = 255;
    else if (speed < -255) speed = -255;
    
    int abs_speed = speed < 0 ? -speed : speed;

    ledcWrite(index, abs_speed);
    
    Spi::data &= ~(1 << INA_BITS[index] | 1 << INB_BITS[index]);
    Spi::data |= (speed >= 0) << INA_BITS[index] | (speed <= 0) << INB_BITS[index];
  }
}