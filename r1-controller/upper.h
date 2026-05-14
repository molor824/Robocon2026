#pragma once

#include "constants.h"
#include "esp32-hal-ledc.h"
#include "shifter.h"

namespace Upper {
  constexpr int CONVEYOR_A_BIT = 8, CONVEYOR_B_BIT = 9, ARM_A_BIT = 10, ARM_B_BIT = 11;
  constexpr int CONVEYOR_PWM_PIN = 27, ARM_PWM_PIN = 14;
  constexpr int CONVEYOR_CHANNEL = 4, ARM_CHANNEL = 5;
  constexpr int MAX_CONVEYOR_SPEED = 100, MAX_ARM_SPEED = 50;

  int conveyorSpeed = 0, armSpeed = 0;

  void sync();
  void setup() {
    ledcSetup(CONVEYOR_CHANNEL, Constant::FREQ, 8);
    ledcSetup(ARM_CHANNEL, Constant::FREQ, 8);
    ledcAttachPin(CONVEYOR_PWM_PIN, CONVEYOR_CHANNEL);
    ledcAttachPin(ARM_PWM_PIN, ARM_CHANNEL);

    sync();
  }

  void sync() {
    Shifter::data &= ~(1 << CONVEYOR_A_BIT | 1 << CONVEYOR_B_BIT | 1 << ARM_A_BIT | 1 << ARM_B_BIT);
    Shifter::data |= (conveyorSpeed >= 0) << CONVEYOR_A_BIT | (conveyorSpeed <= 0) << CONVEYOR_B_BIT
      | (armSpeed >= 0) << ARM_A_BIT | (armSpeed <= 0) << ARM_B_BIT;
    Shifter::sync();
    
    int abs_conveyor_speed = std::abs(conveyorSpeed);
    int abs_arm_speed = std::abs(armSpeed);
    ledcWrite(CONVEYOR_CHANNEL, std::min(abs_conveyor_speed, MAX_CONVEYOR_SPEED));
    ledcWrite(ARM_CHANNEL, std::min(abs_arm_speed, MAX_ARM_SPEED));
  }
}
