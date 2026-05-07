#pragma once

#include <atomic>
#include "motor.h"

namespace Encoder {
  constexpr int A_PINS[Motor::COUNT] = {36, 39, 35, 34};
  constexpr int B_PINS[Motor::COUNT] = {32, 33, 26, 25};

  constexpr int INC_STATES[Motor::COUNT] = {1, 1, 1, 1};

  std::atomic_int counts[Motor::COUNT] = {};

  void IRAM_ATTR isrEncoder(void *arg) {
    int index = *(int *)arg;
    int state = digitalRead(B_PINS[index]);

    if (state == INC_STATES[index]) {
      counts[index].fetch_add(1);
    } else {
      counts[index].fetch_sub(1);
    }
  }

  void setup() {
    for (int i = 0; i < Motor::COUNT; i++) {
      pinMode(A_PINS[i], INPUT);
      pinMode(B_PINS[i], INPUT_PULLUP);

      attachInterruptArg(A_PINS[i], isrEncoder, new int(i), FALLING);
    }
  }
}