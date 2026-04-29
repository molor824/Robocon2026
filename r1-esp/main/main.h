#ifndef __MAIN_H__
#define __MAIN_H__

#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

#define TAG "main"
#define LOGI(format, ...) ESP_LOGI(TAG, format, ##__VA_ARGS__)

#define SPEED_MODE LEDC_LOW_SPEED_MODE
#define PWM_TIMER LEDC_TIMER_0

#endif