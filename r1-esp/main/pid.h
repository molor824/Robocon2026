#pragma once

typedef struct pid {
    float p, i, d;
    float lastError;
    float integral;
} pid_t;

pid_t pid_new(float p, float i, float d) {
    return (pid_t){.p = p, .i = i, .d = d};
}
float pid_correct(pid_t *pid, float error, float dt) {
    float diff = (error - pid->lastError) / dt;
    pid->lastError = error;
    pid->integral += error * dt;
    return pid->p * error + pid->d * diff + pid->i * pid->integral;
}
