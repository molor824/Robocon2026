#ifndef __WHEEL_H__
#define __WHEEL_H__

#include "main.h"

#define WHEEL_COUNT 4

const int wheel_ina_bits[WHEEL_COUNT] = {0, 2, 4, 6};
const int wheel_inb_bits[WHEEL_COUNT] = {1, 3, 5, 7};

const ledc_channel_t wheel_pwm_channels[WHEEL_COUNT] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3};
const gpio_num_t wheel_pwm_pins[WHEEL_COUNT] = {GPIO_NUM_27, GPIO_NUM_14, GPIO_NUM_17, GPIO_NUM_16};

// Motor direction of positive speed, describing INA pin
const int wheel_directions[WHEEL_COUNT] = {1, 1, 1, 1};

int motor_speeds[WHEEL_COUNT];

void wheel_set_motor_speed(int index, int speed) {
    speed *= wheel_directions[index];
    if (speed > 255) speed = 255;
    else if (speed < -255) speed = -255;
    motor_speeds[index] = speed;
}
void wheel_motor_update() {
    spi_data_t data = 0;
    for (int i = 0; i < WHEEL_COUNT; i++) {
        int speed = motor_speeds[i];
        data |= ((speed >= 0) << wheel_ina_bits[i]);
        data |= ((speed <= 0) << wheel_inb_bits[i]);

        // Temporarily disable PWM until spi has sent data
        ESP_ERROR_CHECK(ledc_set_duty(SPEED_MODE, wheel_pwm_channels[i], 0));
        ESP_ERROR_CHECK(ledc_update_duty(SPEED_MODE, wheel_pwm_channels[i]));
    }
    spi_send_data(data);

    for (int i = 0; i < WHEEL_COUNT; i++) {
        int speed = motor_speeds[i];
        int abs_speed = speed < 0 ? -speed : speed;
        ESP_ERROR_CHECK(ledc_set_duty(SPEED_MODE, wheel_pwm_channels[i], abs_speed));
        ESP_ERROR_CHECK(ledc_update_duty(SPEED_MODE, wheel_pwm_channels[i]));
    }
}
void wheel_init() {
    gpio_config_t config = {
        .mode = GPIO_MODE_OUTPUT,
    };

    for (int i = 0; i < WHEEL_COUNT; i++) {
        config.pin_bit_mask |= (1ULL << wheel_pwm_pins[i]);
    }
    ESP_ERROR_CHECK(gpio_config(&config));

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

#endif