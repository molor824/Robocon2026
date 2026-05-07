#pragma once

#include "i2c.h"

namespace Ctl {
  ControllerPtr controller;

  const int maxWaitTime = 1000;
  int lastUpdatedTime;

  void failsafe() {
    for (int i = 0; i < I2C::SERVO_COUNT; i++)
      I2C::servoPositions[i] = 0.0f;
    I2C::sync();
  }

  void onConnectedController(ControllerPtr ctl) {
    Serial.printf("Connecting controller: %s\n", ctl->getModelName().c_str());
    if (!controller) {
      controller = ctl;
      Serial.printf("Connected\n");
    }
  }
  void onDisconnectedController(ControllerPtr ctl) {
    if (controller == ctl) {
      controller = nullptr;
    }
    if (!controller) {
      Serial.println("Disconnected.");
    }
  }

  bool update() {
    bool dataUpdated = BP32.update();
    bool connected = controller && controller->isConnected();
    if (connected && controller->hasData()) {
      lastUpdatedTime = millis();
    } else if (!connected) {
      failsafe();
      return false;
    }

    int currentTime = millis();
    if (currentTime - lastUpdatedTime > maxWaitTime) {
      failsafe();
      return false;
    }

    return true;
  }

  void setup() {
    lastUpdatedTime = millis();

    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.enableVirtualDevice(false);
  }
}