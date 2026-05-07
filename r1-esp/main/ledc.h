#pragma once

#include <driver/ledc.h>

#include "tag.h"

#define SPEED_MODE LEDC_HIGH_SPEED_MODE
#define PWM_TIMER LEDC_TIMER_0

void ledc_init() {
    ledc_timer_config_t timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 20000,
        .speed_mode = SPEED_MODE,
        .timer_num = PWM_TIMER,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    LOGI("LEDC Initialized.");
}