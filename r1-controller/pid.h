#pragma once

struct PID {
  float p, i, d;

  PID() : p(1.0f), i(0.0f), d(0.0f) {}
  PID(float p, float i, float d) : p(p), i(i), d(d) {}

  float correct(float error, float dt) {
    float diff = (error - prev_err) / dt;
    integral += error * dt;
    prev_err = error;
    return error * p + integral * i + diff * d;
  }
private:
  float prev_err;
  float integral;
};