#ifndef __WHEEL_H__
#define __WHEEL_H__

#include "main.h"

#define WHEEL_COUNT 4

const ledc_channel_t wheel_a_channels[WHEEL_COUNT] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3};
const ledc_channel_t wheel_b_channels[WHEEL_COUNT] = {LEDC_CHANNEL_4, LEDC_CHANNEL_5, LEDC_CHANNEL_6, LEDC_CHANNEL_7};

const gpio_num_t wheel_a_pins[WHEEL_COUNT] = {GPIO_NUM_27, GPIO_NUM_22, GPIO_NUM_13, GPIO_NUM_23};
const gpio_num_t wheel_b_pins[WHEEL_COUNT] = {GPIO_NUM_14, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18};

// Motor direction of positive speed, describing INA pin
const int wheel_directions[WHEEL_COUNT] = {1, 1, 1, 1};

void wheel_set_speed(int index, int speed) {
    if (speed == 0) {
        ESP_ERROR_CHECK(ledc_set_duty_and_update(SPEED_MODE, wheel_a_channels[index], 255, 0));
        ESP_ERROR_CHECK(ledc_set_duty_and_update(SPEED_MODE, wheel_b_channels[index], 255, 0));
    } else {
        int abs_speed = speed < 0 ? -speed : speed;
        if (abs_speed > 255) abs_speed = 255;
        int pos_dir = wheel_directions[index];
        ESP_ERROR_CHECK(
            ledc_set_duty_and_update(SPEED_MODE, wheel_a_channels[index], (speed < 0 ? !pos_dir : pos_dir) * abs_speed, 0)
        );
        ESP_ERROR_CHECK(
            ledc_set_duty_and_update(SPEED_MODE, wheel_b_channels[index], (speed < 0 ? pos_dir : !pos_dir) * abs_speed, 0)
        );
    }
}
void wheel_init() {
    gpio_config_t config = {
        .mode = GPIO_MODE_OUTPUT
    };

    for (int i = 0; i < WHEEL_COUNT; i++) {
        config.pin_bit_mask |= (1ULL << wheel_a_pins[i]) | (1ULL << wheel_b_pins[i]);
    }
    ESP_ERROR_CHECK(gpio_config(&config));

    for (int i = 0; i < WHEEL_COUNT; i++) {
        ledc_channel_config_t a_channel = {
            .channel = wheel_a_channels[i],
            .speed_mode = SPEED_MODE,
            .gpio_num = wheel_a_pins[i],
            .timer_sel = PWM_TIMER,
        };
        ledc_channel_config_t b_channel = {
            .channel = wheel_b_channels[i],
            .speed_mode = SPEED_MODE,
            .gpio_num = wheel_b_pins[i],
            .timer_sel = PWM_TIMER,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&a_channel));
        ESP_ERROR_CHECK(ledc_channel_config(&b_channel));
    }

    LOGI("Initialized wheel.");
}

#endif