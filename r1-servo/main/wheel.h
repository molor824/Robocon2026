#pragma once

#include <driver/ledc.h>
#include <driver/gpio.h>

#include "spi.h"
#include "ledc.h"
#include "tag.h"

#define WHEEL_COUNT 4

const int wheel_ina_bits[WHEEL_COUNT] = {0, 2, 6, 4};
const int wheel_inb_bits[WHEEL_COUNT] = {1, 3, 7, 5};

const ledc_channel_t wheel_pwm_channels[WHEEL_COUNT] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3};
const gpio_num_t wheel_pwm_pins[WHEEL_COUNT] = {GPIO_NUM_17, GPIO_NUM_16, GPIO_NUM_27, GPIO_NUM_14};

// Motor direction of positive speed, describing INA pin
const int wheel_directions[WHEEL_COUNT] = {1, -1, 1, 1};

void wheel_set_motor_speed(int index, int speed) {
    speed *= wheel_directions[index];

    int abs_speed = speed < 0 ? -speed : speed;
    if (abs_speed > 255) abs_speed = 255;
    
    spi_data &= ~(1 << wheel_ina_bits[index] | 1 << wheel_inb_bits[index]);
    spi_data |= ((speed >= 0 ? 1 : 0) << wheel_ina_bits[index]) | ((speed <= 0 ? 1 : 0) << wheel_inb_bits[index]);
    ESP_ERROR_CHECK(ledc_set_duty(SPEED_MODE, wheel_pwm_channels[index], abs_speed));
}
void wheel_motor_update() {
    for (int i = 0; i < WHEEL_COUNT; i++) {
        ESP_ERROR_CHECK(ledc_update_duty(SPEED_MODE, wheel_pwm_channels[i]));
    }
    spi_sync();
}
void wheel_init() {
    // gpio_config_t config = {
//     .mode = GPIO_MODE_OUTPUT,        
    // };

    // for (int i = 0; i < WHEEL_COUNT; i++) {
    //     config.pin_bit_mask |= (1ULL << wheel_pwm_pins[i]);
    // }
    // ESP_ERROR_CHECK(gpio_config(&config));

    for (int i = 0; i < WHEEL_COUNT; i++) {
        ledc_channel_config_t config = {
            .channel = wheel_pwm_channels[i],
            .speed_mode = SPEED_MODE,
            .gpio_num = wheel_pwm_pins[i],
            .timer_sel = PWM_TIMER,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&config));
    }

    LOGI("Initialized wheel.");
}
