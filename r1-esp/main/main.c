#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>

#include "main.h"

void set_motor_speed(int index, int speed) {
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
void motor_test(void *arg) {
    // Test 4 motors seperately
    for (;;) {
        for (int i = 0; i < WHEEL_COUNT; i++) {
            LOGI("Motor: %d", i);
            // Sweep from 0 to 255
            for (int s = 0; s < 256; s++) {
                set_motor_speed(i, s);
                vTaskDelay(1000 / 256 / portTICK_PERIOD_MS);
            }
            // Sweep back
            for (int s = 256; s > 0; s--) {
                set_motor_speed(i, s - 1);
                vTaskDelay(1000 / 256 / portTICK_PERIOD_MS);
            }
            // Wait 1 second
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}
void print_encoders(void *arg) {
    for (;;) {
        int64_t counts[WHEEL_COUNT];
        for (int i = 0; i < WHEEL_COUNT; i++) {
            xSemaphoreTake(encoder_mutexes[i], portMAX_DELAY);
            counts[i] = encoder_counts[i];
            xSemaphoreGive(encoder_mutexes[i]);
        }
        LOGI("Encoders: %lld, %lld, %lld, %lld", counts[0], counts[1], counts[2], counts[3]);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
void IRAM_ATTR encoder_isr(void *arg) {
    int index = *(int *)arg;
    uint8_t state = gpio_get_level(encoder_b_pins[index]);

    BaseType_t higherTaskWoken = pdFALSE;
    xQueueSendFromISR(encoder_queues[index], &state, &higherTaskWoken);
    portYIELD_FROM_ISR(higherTaskWoken);
}
void encoder_receiver(void *arg) {
    int index = *(int *)arg;
    for (;;) {
        uint8_t state;
        if (xQueueReceive(encoder_queues[index], &state, portMAX_DELAY)) {
            xSemaphoreTake(encoder_mutexes[index], portMAX_DELAY);
            encoder_counts[index] += state ? encoder_dirs[index] : -encoder_dirs[index];
            xSemaphoreGive(encoder_mutexes[index]);
        }
    }
}
void init_isr() {
    for (int i = 0; i < WHEEL_COUNT; i++) {
        encoder_queues[i] = xQueueCreate(0x1000, sizeof(uint8_t));
        encoder_mutexes[i] = xSemaphoreCreateMutex();
    }

    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    for (int i = 0; i < WHEEL_COUNT; i++) {
        ESP_ERROR_CHECK(gpio_isr_handler_add(encoder_a_pins[i], encoder_isr, (void *)&encoder_args[i]));
        xTaskCreate(encoder_receiver, "Encoder Receiver", 0x1000, (void *)&encoder_args[i], 20, NULL);
    }

    LOGI("Initialized ISR.");
}
void init_gpio() {
    gpio_config_t out_config = {
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config_t in_a_config = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config_t in_b_config = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    for (int i = 0; i < WHEEL_COUNT; i++) {
        out_config.pin_bit_mask |= (1ULL << wheel_a_pins[i]) | (1ULL << wheel_b_pins[i]);
        in_a_config.pin_bit_mask |= (1ULL << encoder_a_pins[i]);
        in_b_config.pin_bit_mask |= (1ULL << encoder_b_pins[i]);
    }
    ESP_ERROR_CHECK(gpio_config(&out_config));
    ESP_ERROR_CHECK(gpio_config(&in_a_config));
    ESP_ERROR_CHECK(gpio_config(&in_b_config));

    LOGI("Initialized GPIO.");
}
void init_pwm() {
    ledc_timer_config_t timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 1000,
        .speed_mode = SPEED_MODE,
        .timer_num = PWM_TIMER,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

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

    ESP_ERROR_CHECK(ledc_fade_func_install(0));

    LOGI("Initialized PWM");
}
void app_main(void)
{
    init_pwm();
    init_gpio();
    init_isr();

    xTaskCreate(print_encoders, "Print encoders", 0x1000, NULL, 1, NULL);
    xTaskCreate(motor_test, "Motor Test", 0x1000, NULL, 2, NULL);
}
