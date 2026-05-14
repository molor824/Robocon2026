#include <VL6180X.h>
#include <Servo.h>

#define TEST_PISTONS

#define WHEEL_COUNT 3
#define LEDC_FREQ 20000
#define SPEED 50
#define WEAPON_THRESHOLD 50
#define STOP_DURATION 1000
#define EXTEND_DURATION 1000
#define GRAB_DURATION 1000
#define LIFT_DURATION 1000
#define RELEASE_DURATION 10000
#define TURN_SPEED 100
#define TURN_DURATION 1000

#define EXTEND_PIN 4
#define GRAB_PIN 16
#define LIFT_PIN 17
#define SERVO_PIN 13

#define MINIFIER 0.6

// #define BLUE_TEAM

Servo weaponServo;
VL6180X sensor;

constexpr int WHEEL_A_PINS[WHEEL_COUNT] = {14, 32, 19};
constexpr int WHEEL_B_PINS[WHEEL_COUNT] = {27, 33, 18};
constexpr int WHEEL_PWM_PINS[WHEEL_COUNT] = {26, 25, 23};
constexpr int WHEEL_CHANNELS[WHEEL_COUNT] = {0, 1, 2};
constexpr float WHEEL_DIRS[WHEEL_COUNT][2] = {
  {-1, 0},
  {0.5, -std::sqrt(2.0) / 3.0},
  {0.5, std::sqrt(2.0) / 3.0}
};

#ifdef BLUE_TEAM
constexpr float INITIAL_DIRECTION[2] = {0.5f, -std::sqrt(2.0f) / 3.0f};
#else
constexpr float INITIAL_DIRECTION[2] = {-0.5f, -std::sqrt(2.0f) / 3.0f};
#endif

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

  sensor.init();
  sensor.configureDefault();
  sensor.setTimeout(100);

  weaponServo.attach(SERVO_PIN);
  weaponServo.write(0);

  delay(1000); // Wait for startup

  velocity[0] = INITIAL_DIRECTION[0] * SPEED;
  velocity[1] = INITIAL_DIRECTION[1] * SPEED;

  wheelUpdate();
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
  int range = sensor.readRangeSingleMillimeters();
  if (range <= WEAPON_THRESHOLD) {
    float oldVelocity[2] = {velocity[0], velocity[1]};
    // Assume weapon is found
    velocity[0] = velocity[1] = rotation = 0;
    wheelUpdate();
    // Wait some time to stop the motors
    delay(STOP_DURATION);
    // Check if the range is still within the threshold, else, assume it has went past
    range = sensor.readRangeSingleMillimeters();
    if (range > WEAPON_THRESHOLD) {
      velocity[0] = -oldVelocity[0] * MINIFIER;
      velocity[1] = oldVelocity[1];
      return;
    }
    // In this case, start grabbing weapon and stop
    digitalWrite(EXTEND_PIN, HIGH);
    delay(EXTEND_DURATION);

    digitalWrite(GRAB_PIN, HIGH);
    delay(GRAB_DURATION);

    digitalWrite(LIFT_PIN, HIGH);
    delay(LIFT_DURATION);

    rotation = TURN_SPEED;
    wheelUpdate();
    delay(TURN_DURATION);

    rotation = 0;
    wheelUpdate();

    weaponServo.write(90);
    delay(RELEASE_DURATION);

    digitalWrite(GRAB_PIN, LOW);
    delay(GRAB_DURATION);

    digitalWrite(EXTEND_PIN, LOW);
    delay(EXTEND_DURATION);

    digitalWrite(LIFT_PIN, LOW);
    vTaskDelete(NULL);
  }
#endif
}
