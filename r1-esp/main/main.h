#ifndef __MAIN_H__
#define __MAIN_H__

#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/spi_master.h>

#define TAG "main"
#define LOGI(format, ...) ESP_LOGI(TAG, format, ##__VA_ARGS__)

#define SPEED_MODE LEDC_HIGH_SPEED_MODE
#define PWM_TIMER LEDC_TIMER_0

#define SPI_MOSI GPIO_NUM_23
#define SPI_SCK GPIO_NUM_18
#define SPI_CS GPIO_NUM_19

typedef uint8_t spi_data_t;

spi_device_handle_t spi;

void spi_send_data(spi_data_t data) {
    spi_transaction_t transaction = {
        .length = sizeof(spi_data_t) * 8,
        .tx_buffer = &data,
    };
    ESP_ERROR_CHECK(spi_device_transmit(spi, &transaction));
}

void spi_init() {
    spi_bus_config_t config = {
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = SPI_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = sizeof(spi_data_t),
    };

    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = 1000000,
        .spics_io_num = SPI_CS,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &config, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_config, &spi));
}

#endif