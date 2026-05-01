#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>

#include "main.h"
#include "wheel.h"
#include "encoder.h"

void motor_test(void *arg) {
    // Test 4 motors
    for (;;) {
        // Sweep from 0 to 255
        for (int s = 0; s < 256; s++) {
            for (int i = 0; i < WHEEL_COUNT; i++)
                wheel_set_speed(i, s);
            vTaskDelay(1000 / 256 / portTICK_PERIOD_MS);
        }
        // Sweep back
        for (int s = 256; s > 0; s--) {
            for (int i = 0; i < WHEEL_COUNT; i++)
                wheel_set_speed(i, s - 1);
            vTaskDelay(1000 / 256 / portTICK_PERIOD_MS);
        }
        // Wait 1 second
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void print_encoders(void *arg) {
    for (;;) {
        int counts[WHEEL_COUNT];
        for (int i = 0; i < WHEEL_COUNT; i++) {
            counts[i] = atomic_load(&encoder_counts[i]);
        }
        LOGI("Encoders: %d, %d, %d, %d", counts[0], counts[1], counts[2], counts[3]);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
void app_main(void)
{
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    ledc_timer_config_t timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 1000,
        .speed_mode = SPEED_MODE,
        .timer_num = PWM_TIMER,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    wheel_init();
    encoder_init();

    xTaskCreate(print_encoders, "Print encoders", 0x1000, NULL, 1, NULL);
    xTaskCreate(motor_test, "Motor Test", 0x1000, NULL, 2, NULL);
}
