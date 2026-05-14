#include <Bluepad32.h>

#include "controller.h"
#include "i2c.h"
#include "wheel.h"
#include "shifter.h"
#include "upper.h"
#include "cylinder.h"

// #define TEST_MOTOR_ORDER
// #define TEST_ENCODER

constexpr int MOTOR_ORDERS[Constant::WHEEL_COUNT] = {2, 3, 1, 0};
constexpr int ROT_DIRECTIONS[Constant::WHEEL_COUNT][2] = {{1, -1}, {1, 1}, {-1, 1}, {-1, -1}};
constexpr float SPEED_MULTIPLIER = 0.5f;
constexpr float ROT_MULTIPLIER = 0.25f;
constexpr float MAX_SPEED = 300.0f;
constexpr float MIN_ACCEPTABLE_SPEED = 50.0f;

uint32_t elapsed;

#ifdef TEST_MOTOR_ORDER
void testMotorOrder(float dt) {
  constexpr float TIMER_DURATION = 4.0f;
  static float timer = 0;
  static int motorIndex = 0;

  int motor = MOTOR_ORDERS[motorIndex % Constant::WHEEL_COUNT];

  for (int i = 0; i < Constant::WHEEL_COUNT; i++) {
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

int prevDpad;
int prevX, prevY, prevA;

void servoControl() {
  int cx = Ctl::controller->axisRX();
  int cy = -Ctl::controller->axisRY();
  int crot = Ctl::controller->axisX();

  bool reset = Ctl::controller->b();

  if (reset) {
    for (int i = 0; i < Constant::WHEEL_COUNT; i++) {
      Wheel::escs[i].writeMicroseconds(Wheel::MINIMUM_MS);
      Wheel::servoRadians[i] = 0.0f;
      I2C::servoPositions[i] = 0;
    }
    I2C::sync();
  } else {
    for (int i = 0; i < Constant::WHEEL_COUNT; i++) {
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

  int brake = Ctl::controller->brake();
  int throttle = Ctl::controller->throttle();
  bool r1 = Ctl::controller->r1() != 0;
  bool l1 = Ctl::controller->l1() != 0;
  Upper::conveyorSpeed = (l1 - r1) * Upper::MAX_CONVEYOR_SPEED;
  Upper::armSpeed = (throttle - brake) * Upper::MAX_ARM_SPEED / 1020;
  Upper::sync();

  int dpad = Ctl::controller->dpad();
  int btnX = Ctl::controller->x();
  int btnY = Ctl::controller->y();
  int btnA = Ctl::controller->a();

  if (btnA && !prevA) Cylinder::grab = !Cylinder::grab;
  if (btnX && !prevX) Cylinder::extend = !Cylinder::extend;
  if (btnY && !prevY) Cylinder::raise = !Cylinder::raise;

  if ((dpad & 1) != 0 && (prevDpad & 1) == 0) Cylinder::wgrab2 = !Cylinder::wgrab2;
  if ((dpad & 2) != 0 && (prevDpad & 2) == 0) Cylinder::wgrab1 = !Cylinder::wgrab1;
  if ((dpad & 4) != 0 && (prevDpad & 4) == 0) Cylinder::wtilt = !Cylinder::wtilt;
  if ((dpad & 8) != 0 && (prevDpad & 8) == 0) Cylinder::wlift = !Cylinder::wlift;

  Cylinder::sync();

  prevDpad = dpad;
  prevA = btnA;
  prevX = btnX;
  prevY = btnY;
}

void setup() {
  Serial.begin(115200);
  Wheel::setup();
  Ctl::setup();
  I2C::setup();
  Shifter::setup();
  Upper::setup();
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
