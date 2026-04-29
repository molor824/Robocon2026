#ifndef __ISR_H__
#define __ISR_H__

#include "main.h"
#include "wheel.h"

#include <stdatomic.h>

const gpio_num_t encoder_a_pins[WHEEL_COUNT] = {GPIO_NUM_36, GPIO_NUM_39, GPIO_NUM_34, GPIO_NUM_35};
const gpio_num_t encoder_b_pins[WHEEL_COUNT] = {GPIO_NUM_32, GPIO_NUM_33, GPIO_NUM_25, GPIO_NUM_26};

const int encoder_inc_states[WHEEL_COUNT] = {1, 1, 1, 1};
const int encoder_args[WHEEL_COUNT] = {0, 1, 2, 3};

atomic_int encoder_counts[WHEEL_COUNT] = {0};

void IRAM_ATTR encoder_isr(void *arg) {
    int index = *(int *)arg;
    int state = gpio_get_level(encoder_b_pins[index]);

    if (state == encoder_inc_states[index]) {
        atomic_fetch_add(&encoder_counts[index], 1);
    } else {
        atomic_fetch_sub(&encoder_counts[index], 1);
    }
}
void encoder_init() {
    gpio_config_t in_a_config = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config_t in_b_config = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };

    for (int i = 0; i < WHEEL_COUNT; i++) {
        in_a_config.pin_bit_mask |= (1ULL << encoder_a_pins[i]);
        in_b_config.pin_bit_mask |= (1ULL << encoder_b_pins[i]);
    }

    ESP_ERROR_CHECK(gpio_config(&in_a_config));
    ESP_ERROR_CHECK(gpio_config(&in_b_config));

    for (int i = 0; i < WHEEL_COUNT; i++) {
        ESP_ERROR_CHECK(gpio_isr_handler_add(encoder_a_pins[i], encoder_isr, (void *)&encoder_args[i]));
    }

    LOGI("Initialized Encoder.");
}

#endif
