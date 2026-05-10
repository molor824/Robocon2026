#pragma once

#include "driver/spi_master.h"

#define SPI_MOSI 23
#define SPI_SS 5
#define SPI_SCK 18
#define SPI_FREQ 10000000

spi_device_handle_t wheel_spi;

uint16_t spi_data;

void spi_sync() {
    spi_transaction_t trans = {
        .length = sizeof(spi_data) * 8,
        .tx_buffer = &spi_data,
    };
    ESP_ERROR_CHECK(spi_device_transmit(wheel_spi, &trans));
}
void spi_init() {
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SPI_MOSI,
        .sclk_io_num = SPI_SCK,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
        .miso_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = sizeof(spi_data),
    };
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = SPI_FREQ,
        .mode = 0,
        .spics_io_num = SPI_SS,
        .queue_size = 1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &dev_cfg, &wheel_spi));
}
