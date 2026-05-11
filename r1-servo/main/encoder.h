#pragma once

#include <stdatomic.h>
#include <driver/gpio.h>
#include <driver/pulse_cnt.h>

#include "wheel.h"

const gpio_num_t encoder_a_pins[WHEEL_COUNT] = {GPIO_NUM_39, GPIO_NUM_35, GPIO_NUM_33, GPIO_NUM_26};
const gpio_num_t encoder_b_pins[WHEEL_COUNT] = {GPIO_NUM_36, GPIO_NUM_34, GPIO_NUM_32, GPIO_NUM_25};

pcnt_unit_handle_t encoder_units[WHEEL_COUNT] = {};
atomic_int encoder_offsets[WHEEL_COUNT] = {};

const int encoder_directions[WHEEL_COUNT] = {-1, -1, -1, -1};

bool IRAM_ATTR encoder_overflow_cb(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *data, void *arg) {
    int index = *(int *)arg;
    atomic_fetch_add(&encoder_offsets[index], data->watch_point_value);
    return false;
}

void encoder_init() {
    pcnt_unit_config_t pcnt_cfg = {
        .low_limit = INT16_MIN,
        .high_limit = INT16_MAX,
    };

    for (int i = 0; i < WHEEL_COUNT; i++) {
        ESP_ERROR_CHECK(pcnt_new_unit(&pcnt_cfg, &encoder_units[i]));
        
        ESP_ERROR_CHECK(pcnt_unit_add_watch_point(encoder_units[i], INT16_MAX));
        ESP_ERROR_CHECK(pcnt_unit_add_watch_point(encoder_units[i], INT16_MIN));

        pcnt_event_callbacks_t cbs = { .on_reach = encoder_overflow_cb };
        int *arg = malloc(sizeof(int));
        *arg = i;
        ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(encoder_units[i], &cbs, (void *)arg));

        pcnt_channel_handle_t channel;
        pcnt_chan_config_t channel_cfg = {
            .edge_gpio_num = encoder_a_pins[i],
            .level_gpio_num = encoder_b_pins[i],
        };
        ESP_ERROR_CHECK(pcnt_new_channel(encoder_units[i], &channel_cfg, &channel));

        ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
            channel,
            PCNT_CHANNEL_EDGE_ACTION_INCREASE,
            PCNT_CHANNEL_EDGE_ACTION_DECREASE
        ));
        ESP_ERROR_CHECK(pcnt_channel_set_level_action(
            channel,
            PCNT_CHANNEL_LEVEL_ACTION_INVERSE,
            PCNT_CHANNEL_LEVEL_ACTION_KEEP
        ));

        ESP_ERROR_CHECK(pcnt_unit_enable(encoder_units[i]));
        ESP_ERROR_CHECK(pcnt_unit_clear_count(encoder_units[i]));
        ESP_ERROR_CHECK(pcnt_unit_start(encoder_units[i]));
    }

    LOGI("Initialized Encoder.");
}

int encoder_get_count(int index) {
    int count = 0;
    ESP_ERROR_CHECK(pcnt_unit_get_count(encoder_units[index], &count));
    return (atomic_load(&encoder_offsets[index]) + count) * encoder_directions[index];
}
