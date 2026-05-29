#include <Servo.h>
#include <Bluepad32.h>
#include "controller.h"

// #define TEST_PISTONS

#define WHEEL_COUNT 3
#define LEDC_FREQ 20000

#define EXTEND_PIN 4
#define GRAB_PIN 16
#define LIFT_PIN 17
#define SERVO_PIN 13

#define MINIFIER 0.6

Servo weaponServo;
ControllerPtr controller = 0;

constexpr int WHEEL_A_PINS[WHEEL_COUNT] = {14, 32, 19};
constexpr int WHEEL_B_PINS[WHEEL_COUNT] = {27, 33, 18};
constexpr int WHEEL_PWM_PINS[WHEEL_COUNT] = {26, 25, 23};
constexpr int WHEEL_CHANNELS[WHEEL_COUNT] = {0, 1, 2};
constexpr float WHEEL_DIRS[WHEEL_COUNT][2] = {
  {-1, 0},
  {0.5, -std::sqrt(3.0) * 0.5},
  {0.5, std::sqrt(3.0) * 0.5}
};
constexpr int MOVE_THRESHOLD = 10;
constexpr float MOVE_MULTIPLIER = 0.5;
constexpr float ROT_MULTIPLIER = 0.5;

float velocity[2] = {};
float rotation;

void setWheelSpeed(int index, int speed) {
  int absSpeed = std::min(std::abs(speed), 255);
  ledcWrite(WHEEL_CHANNELS[index], absSpeed);
  digitalWrite(WHEEL_A_PINS[index], speed >= 0);
  digitalWrite(WHEEL_B_PINS[index], speed <= 0);
}
void wheelUpdate() {
  for (int i = 0; i < WHEEL_COUNT; i++) {
    float speed = velocity[0] * WHEEL_DIRS[i][0] + velocity[1] * WHEEL_DIRS[i][1] + rotation;
    setWheelSpeed(i, speed);
  }
}

void failsafe() {
  for (int i = 0; i < WHEEL_COUNT; i++) {
    digitalWrite(WHEEL_A_PINS[i], 0);
    digitalWrite(WHEEL_B_PINS[i], 0);
    ledcWrite(WHEEL_CHANNELS[i], 0);
  }
}

void setup() {
  pinMode(EXTEND_PIN, OUTPUT);
  pinMode(GRAB_PIN, OUTPUT);
  pinMode(LIFT_PIN, OUTPUT);

  for (int i = 0; i < WHEEL_COUNT; i++) {
    ledcSetup(WHEEL_CHANNELS[i], LEDC_FREQ, 8);
    ledcAttachPin(WHEEL_PWM_PINS[i], WHEEL_CHANNELS[i]);
    pinMode(WHEEL_A_PINS[i], OUTPUT);
    pinMode(WHEEL_B_PINS[i], OUTPUT);
  }

  weaponServo.attach(SERVO_PIN);
  weaponServo.write(0);

  wheelUpdate();

  Ctl::setup();
}

void loop() {
#ifdef TEST_PISTONS
  digitalWrite(EXTEND_PIN, 1);
  delay(1000);
  digitalWrite(EXTEND_PIN, 0);
  delay(1000);
  digitalWrite(GRAB_PIN, 1);
  delay(1000);
  digitalWrite(GRAB_PIN, 0);
  delay(1000);
  digitalWrite(LIFT_PIN, 1);
  delay(1000);
  digitalWrite(LIFT_PIN, 0);
  delay(1000);
#else
  if (Ctl::update()) {
    int xi = Ctl::controller->axisRX();
    int yi = Ctl::controller->axisRY();
    int roti = Ctl::controller->axisX();

    if (abs(xi) < MOVE_THRESHOLD) xi = 0;
    if (abs(yi) < MOVE_THRESHOLD) yi = 0;
    if (abs(roti) < MOVE_THRESHOLD) roti = 0;

    velocity[0] = (float)xi * MOVE_MULTIPLIER;
    velocity[1] = (float)yi * MOVE_MULTIPLIER;
    rotation = (float)roti * ROT_MULTIPLIER;
    wheelUpdate();
  }

  delay(10);
#endif
}
