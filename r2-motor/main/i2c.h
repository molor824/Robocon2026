#pragma once

#ifndef WHEEL_COUNT
#error "WHEEL_COUNT should be defined before use!"
#endif

#include "freertos/FreeRTOS.h"
#include "string.h"
#include "driver/i2c_slave.h"
#include "driver/gpio.h"

#define I2C_SCL 22
#define I2C_SDA 21
#define I2C_ADDR 69
#define I2C_BUF_LEN 0x100

i2c_slave_dev_handle_t i2c;

portMUX_TYPE i2c_lock = portMUX_INITIALIZER_UNLOCKED;
int i2c_shared_speeds[WHEEL_COUNT];

SemaphoreHandle_t i2c_data_received;

bool i2c_slave_receive_cb(i2c_slave_dev_handle_t, const i2c_slave_rx_done_event_data_t *evt, void *) {
    if (evt->length < sizeof(i2c_shared_speeds)) return false;
    portENTER_CRITICAL_ISR(&i2c_lock);
    memcpy(i2c_shared_speeds, evt->buffer, sizeof(i2c_shared_speeds));
    portEXIT_CRITICAL_ISR(&i2c_lock);

    BaseType_t taskWoken = 0;
    xSemaphoreGiveFromISR(i2c_data_received, &taskWoken);
    return taskWoken;
}

void i2c_get_speeds(int speeds[WHEEL_COUNT]) {
    xSemaphoreTake(i2c_data_received, portMAX_DELAY);
    portENTER_CRITICAL(&i2c_lock);
    memcpy(speeds, i2c_shared_speeds, sizeof(i2c_shared_speeds));
    portEXIT_CRITICAL(&i2c_lock);
}

void i2c_init() {
    i2c_slave_config_t slave_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .slave_addr = I2C_ADDR,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .send_buf_depth = I2C_BUF_LEN,
        .receive_buf_depth = I2C_BUF_LEN,
        .addr_bit_len = I2C_ADDR_BIT_LEN_7,
    };
    ESP_ERROR_CHECK(i2c_new_slave_device(&slave_cfg, &i2c));
    i2c_slave_event_callbacks_t cbs = {
        .on_receive = i2c_slave_receive_cb,
    };
    ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(i2c, &cbs, NULL));
}
