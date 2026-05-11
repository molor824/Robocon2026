#include <Bluepad32.h>
#include "controller.h"
#include "i2c.h"
#include "wheel.h"

// #define TEST_MOTOR_ORDER
// #define TEST_ENCODER

constexpr int CENTER_X = 4, CENTER_Y = 4;
constexpr int MOTOR_ORDERS[I2C::COUNT] = {3, 0, 1, 2};
constexpr int ROT_DIRECTIONS[I2C::COUNT][2] = {{1, -1}, {-1, -1}, {-1, 1}, {1, 1}};
constexpr float SPEED_MULTIPLIER = 0.2f;
constexpr float ROT_MULTIPLIER = 0.1f;
constexpr float MAX_SPEED = 100.0f;
constexpr float MIN_ACCEPTABLE_SPEED = 16.0f;

uint32_t elapsed;

#ifdef TEST_MOTOR_ORDER
void testMotorOrder(float dt) {
  constexpr float TIMER_DURATION = 4.0f;
  static float timer = 0;
  static int motorIndex = 0;

  int motor = MOTOR_ORDERS[motorIndex % Wheel::COUNT];

  for (int i = 0; i < Wheel::COUNT; i++) {
    // I2C::servoPositions[i] = i == motor ? (motorIndex % (Wheel::COUNT * 2) < Wheel::COUNT ? 180 : -180) : 0;
    I2C::servoPositions[i] = timer < (TIMER_DURATION / 2) ? 0 : 360;
  }
  I2C::sync();

  timer += dt;
  if (timer >= TIMER_DURATION) {
    timer -= TIMER_DURATION;
    motorIndex++;
  }
}
#endif

#ifdef TEST_ENCODER
int getDigit(char ch) {
  if (ch < '0' || ch > '9') return -1;
  return ch - '0';
}
void testEncoder() {
  String line = Serial.readStringUntil('\n');
  int index = 0;
  for (; index < line.length();) {
    int motor = 0;
    bool parsed = false;
    for (; index < line.length();) {
      int digit = getDigit(line[index]);
      index++;
      if (digit == -1) {
        if (!parsed) continue;
        else break;
      }
      parsed = true;
      motor = motor * 10 + digit;
    }
    if (!parsed) break;
    int count = 0;
    parsed = false;
    for (; index < line.length();) {
      int digit = getDigit(line[index]);
      index++;
      if (digit == -1) {
        if (!parsed) continue;
        else break;
      }
      parsed = true;
      count = count * 10 + digit;
    }
    if (!parsed) break;
    if (motor >= 4) {
      Serial.printf("Motor index cannot be more than 3.\n");
      continue;
    }
    I2C::servoPositions[motor] = count;
  }
  I2C::sync();
  Serial.println("Synced.\n");
}
#endif

bool resetState = false;

void servoControl() {
  int cx = Ctl::controller->axisRX() - CENTER_X;
  int cy = -(Ctl::controller->axisRY() - CENTER_Y);
  int crot = Ctl::controller->axisX() - CENTER_X;

  bool currentResetState = Ctl::controller->b();
  bool resetPressed = currentResetState && !resetState;
  resetState = currentResetState;

  if (resetState) {
    for (int i = 0; i < I2C::COUNT; i++) {
      int motor = MOTOR_ORDERS[i];
      Wheel::escs[motor].writeMicroseconds(Wheel::MINIMUM_MS);
      Wheel::servoRadians[motor] = 0.0f;
      I2C::servoPositions[motor] = 0;
    }
    I2C::sync();
  } else {
    for (int i = 0; i < I2C::COUNT; i++) {
      int motor = MOTOR_ORDERS[i];
      float x = cx * SPEED_MULTIPLIER + ROT_DIRECTIONS[i][0] * ROT_MULTIPLIER * crot;
      float y = cy * SPEED_MULTIPLIER + ROT_DIRECTIONS[i][1] * ROT_MULTIPLIER * crot;

      float targetRadian = atan2f(-x, y);
      float magnitude = sqrtf(x * x + y * y);

      if (magnitude > MAX_SPEED) magnitude = MAX_SPEED;

      Wheel::escs[motor].writeMicroseconds(roundf(Wheel::MINIMUM_MS + magnitude));
      if (magnitude >= MIN_ACCEPTABLE_SPEED)
        Wheel::servoRadians[motor] = targetRadian;
    }
    Wheel::sync();
  }
}

void setup() {
  Serial.begin(115200);
  Wheel::setup();
  Ctl::setup();
  I2C::setup();
  elapsed = millis();
}

void loop() {
  delay(10);
  uint32_t current = millis();
  uint32_t diff = current - elapsed;
  elapsed = current;

  float dt = (float)diff * 0.001f;

#ifndef TEST_ENCODER
#ifndef TEST_MOTOR_ORDER
  if (Ctl::update()) {
    servoControl();
  }
#else
    testMotorOrder(dt);
#endif
#else
  testEncoder();
#endif
}
