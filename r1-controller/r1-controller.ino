#include <Bluepad32.h>
#include "controller.h"
#include "i2c.h"

constexpr int CENTER_X = 4, CENTER_Y = 4;

void servoControl() {
  int x = Ctl::controller->axisX() - CENTER_X;
  int y = Ctl::controller->axisY() - CENTER_Y;

  float targetRadian = atan2f(-y, x);
  
  for (int i = 0; i < I2C::SERVO_COUNT; i++) {
    I2C::servoPositions[i] = targetRadian;
  }
  I2C::sync();
}

void setup() {
  Serial.begin(115200);
  I2C::setup();
}

void loop() {
  delay(1);
  if (Ctl::update()) {
    servoControl();
  }
}
