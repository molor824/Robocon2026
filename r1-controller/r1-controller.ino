#include <Bluepad32.h>
#include "spi.h"
#include "motor.h"
#include "encoder.h"
#include "pid.h"
#include "controller.h"

constexpr int FULL_ROT_COUNT = 1000 * 5;
constexpr int CENTER_X = 4, CENTER_Y = 4;

constexpr float P = 100.0f;
constexpr float I = 0.0f;
constexpr float D = 0.0f;

PID motor_pids[Motor::COUNT];

int lastElapsedTime;
float deltaTime;

void servoControl() {
  int x = Ctl::controller->axisX() - CENTER_X;
  int y = Ctl::controller->axisY() - CENTER_Y;

  float targetRadian = atan2f(-y, x);
  
  Serial.print("Motor: ");
  for (int i = 0; i < Motor::COUNT; i++) {
    int encoderCount = Encoder::counts[i].load();
    float measuredRadian = (float)encoderCount / (float)FULL_ROT_COUNT * (2.0f * PI);
    float error = targetRadian - measuredRadian;
    int correction = (int)roundf(motor_pids[i].correct(error, deltaTime));
    if (i != 0) Serial.print(", ");
    Serial.print(correction);
    Motor::setSpeed(i, correction);
  }
  Serial.printf("; %x\n", Spi::data);
  Spi::sync();
}
void motorTest() {
  int time = lastElapsedTime % 4000;
  for (int i = 0; i < Motor::COUNT; i++) {
    Motor::setSpeed(i, time < 2000 ? 255 : -255);
  }
  Serial.println(Spi::data, HEX);
  Spi::sync();
}

void setup() {
  Serial.begin(115200);
  Spi::setup();
  Motor::setup();
  Encoder::setup();
  Ctl::setup();

  lastElapsedTime = millis();

  for (int i = 0; i < Motor::COUNT; i++) {
    motor_pids[i] = PID(P, I, D);
  }
}

void loop() {
  delay(10);
  int diff = millis() - lastElapsedTime;
  lastElapsedTime += diff;
  deltaTime = (float)diff / 1000.0f;

  // if (Ctl::update()) {
  //   servoControl();
  // }
  motorTest();
}
