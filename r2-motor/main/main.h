#pragma once

#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "driver/i2c_slave.h"
#include "spi.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"


#define WHEEL_COUNT 6
#include "i2c.h"

#define WHEEL_TIMER LEDC_TIMER_0
#define WHEEL_SPEED_MODE LEDC_HIGH_SPEED_MODE

const int WHEEL_CHANNELS[WHEEL_COUNT] = {0, 1, 2, 3, 4, 5};
const int WHEEL_PINS[WHEEL_COUNT] = {33, 32, 26, 25, 14, 27};

const int WHEEL_INA_BITS[WHEEL_COUNT] = {0, 2, 4, 6, 9, 8};
const int WHEEL_INB_BITS[WHEEL_COUNT] = {1, 3, 5, 7, 11, 10};

bool parse_wheel_speeds(int speeds[WHEEL_COUNT]) {
    int values[WHEEL_COUNT];
    int parsed = scanf("%d %d %d %d %d %d",
                        &values[0], &values[1], &values[2],
                        &values[3], &values[4], &values[5]);

    if (parsed != WHEEL_COUNT) {
        printf("Error: expected 6 values, got %d\n", parsed);
        return false;
    }

    for (int i = 0; i < WHEEL_COUNT; i++) {
        speeds[i] = values[i];
    }

    return true;
}

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

    uart_driver_install(UART_NUM_0, 256, 256, 0, NULL, 0);
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
}
void wheel_set_speed(int index, int speed) {
    int abs_speed = speed < 0 ? -speed : speed;
    if (abs_speed > 255) abs_speed = 255;

    ESP_ERROR_CHECK(ledc_set_duty(WHEEL_SPEED_MODE, WHEEL_CHANNELS[index], abs_speed));
    spi_data &= ~(1 << WHEEL_INA_BITS[index] | 1 << WHEEL_INB_BITS[index]);
    spi_data |= (speed >= 0) << WHEEL_INA_BITS[index] | (speed <= 0) << WHEEL_INB_BITS[index];
}
void wheel_sync() {
    for (int i = 0; i < WHEEL_COUNT; i++) {
        ESP_ERROR_CHECK(ledc_update_duty(WHEEL_SPEED_MODE, WHEEL_CHANNELS[i]));
    }
    spi_sync();
}

void wheel_task() {
    for (;;) {
        int speeds[WHEEL_COUNT] = {};
        // i2c_get_speeds(speeds, pdMS_TO_TICKS(1000));
        if (!parse_wheel_speeds(speeds)) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        printf("speed: ");
        for (int i = 0; i < WHEEL_COUNT; i++) {
            printf("%d, ", speeds[i]);
            wheel_set_speed(i, speeds[i]);
        }
        printf("\n");
        wheel_sync();
    }
}
