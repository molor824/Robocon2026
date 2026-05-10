#pragma once

#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "driver/i2c_slave.h"
#include "spi.h"

#define WHEEL_COUNT 6
#include "i2c.h"

#define WHEEL_TIMER LEDC_TIMER_0
#define WHEEL_SPEED_MODE LEDC_HIGH_SPEED_MODE

const int WHEEL_CHANNELS[WHEEL_COUNT] = {0, 1, 2, 3, 4, 5};
const int WHEEL_PINS[WHEEL_COUNT] = {32, 33, 25, 26, 27, 14};

const int WHEEL_INA_PINS[WHEEL_COUNT] = {0, 2, 4, 6, 8, 10};
const int WHEEL_INB_PINS[WHEEL_COUNT] = {1, 3, 5, 7, 9, 11};

void init() {
    spi_init();
    i2c_init();
    ledc_timer_config_t timer_config = {
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = 0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 20000,
        .speed_mode = WHEEL_SPEED_MODE,
        .timer_num = WHEEL_TIMER,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    for (int i = 0; i < WHEEL_COUNT; i++) {
        ledc_channel_config_t channel_config = {
            .channel = WHEEL_CHANNELS[i],
            .gpio_num = WHEEL_PINS[i],
            .speed_mode = WHEEL_SPEED_MODE,
            .timer_sel = WHEEL_TIMER,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
    }
}
void wheel_set_speed(int index, int speed) {
    int abs_speed = speed < 0 ? -speed : speed;
    if (abs_speed > 255) abs_speed = 255;

    ESP_ERROR_CHECK(ledc_set_duty(WHEEL_SPEED_MODE, WHEEL_CHANNELS[index], abs_speed));
    spi_data &= ~(1 << WHEEL_INA_PINS[index] | 1 << WHEEL_INB_PINS[index]);
    spi_data |= (speed >= 0) << WHEEL_INA_PINS[index] | (speed <= 0) << WHEEL_INB_PINS[index];
}
void wheel_sync() {
    for (int i = 0; i < WHEEL_COUNT; i++) {
        ESP_ERROR_CHECK(ledc_update_duty(WHEEL_SPEED_MODE, WHEEL_CHANNELS[i]));
    }
    spi_sync();
}

void wheel_task() {
    for (;;) {
        int speeds[WHEEL_COUNT];
        i2c_get_speeds(speeds);
        for (int i = 0; i < WHEEL_COUNT; i++) {
            wheel_set_speed(i, speeds[i]);
        }
        wheel_sync();
    }
}
