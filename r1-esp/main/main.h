#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>

#define TAG "main"
#define LOGI(format, ...) ESP_LOGI(TAG, format, ##__VA_ARGS__)

#define SPEED_MODE LEDC_LOW_SPEED_MODE
#define PWM_TIMER LEDC_TIMER_0

#define WHEEL_COUNT 4

const ledc_channel_t wheel_a_channels[WHEEL_COUNT] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3};
const ledc_channel_t wheel_b_channels[WHEEL_COUNT] = {LEDC_CHANNEL_4, LEDC_CHANNEL_5, LEDC_CHANNEL_6, LEDC_CHANNEL_7};

const gpio_num_t wheel_a_pins[WHEEL_COUNT] = {GPIO_NUM_27, GPIO_NUM_22, GPIO_NUM_13, GPIO_NUM_23};
const gpio_num_t wheel_b_pins[WHEEL_COUNT] = {GPIO_NUM_14, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18};

const gpio_num_t encoder_a_pins[WHEEL_COUNT] = {GPIO_NUM_36, GPIO_NUM_39, GPIO_NUM_34, GPIO_NUM_35};
const gpio_num_t encoder_b_pins[WHEEL_COUNT] = {GPIO_NUM_32, GPIO_NUM_33, GPIO_NUM_25, GPIO_NUM_26};

// Motor direction of positive speed, describing INA pin
const int wheel_directions[WHEEL_COUNT] = {1, 1, 1, 1};

const int encoder_dirs[WHEEL_COUNT] = {1, 1, 1, 1};

const int encoder_args[WHEEL_COUNT] = {0, 1, 2, 3};

QueueHandle_t encoder_queues[WHEEL_COUNT];
SemaphoreHandle_t encoder_mutexes[WHEEL_COUNT];
int64_t encoder_counts[WHEEL_COUNT];

#endif