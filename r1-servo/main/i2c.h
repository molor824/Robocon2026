#pragma once

#include <stdint.h>
#include <string.h>
#include <driver/i2c_slave.h>

#include "wheel.h"
#include "tag.h"

#define I2C_ADDRESS 69
#define I2C_BUF 1024

// -PI, PI
int shared_servo_positions[WHEEL_COUNT];
portMUX_TYPE servo_lock = portMUX_INITIALIZER_UNLOCKED;
i2c_slave_dev_handle_t i2c;

bool i2c_recv_cb(i2c_slave_dev_handle_t handle, const i2c_slave_rx_done_event_data_t *evt, void *arg) {
    if (evt->length < sizeof(shared_servo_positions)) return false;

    portENTER_CRITICAL_ISR(&servo_lock);
    memcpy(shared_servo_positions, evt->buffer, sizeof(shared_servo_positions));
    portEXIT_CRITICAL_ISR(&servo_lock);

    return false;
}

void i2c_init() {
    i2c_slave_config_t conf = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .slave_addr = I2C_ADDRESS,
        .receive_buf_depth = I2C_BUF,
        .send_buf_depth = I2C_BUF,
        .addr_bit_len = I2C_ADDR_BIT_LEN_7,  // 7-bit address
    };
    ESP_ERROR_CHECK(i2c_new_slave_device(&conf, &i2c));

    i2c_slave_event_callbacks_t cbs = {
        .on_receive = i2c_recv_cb,
    };
    ESP_ERROR_CHECK(i2c_slave_register_event_callbacks(i2c, &cbs, NULL));

    LOGI("I2C Initialized.");
}

void i2c_read_servo_positions(int *positions) {
    portENTER_CRITICAL(&servo_lock);
    memcpy(positions, shared_servo_positions, sizeof(shared_servo_positions));
    portEXIT_CRITICAL(&servo_lock);
}
