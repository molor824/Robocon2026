#pragma once

#include <Bluepad32.h>
#include "esp32-hal-spi.h"
#include "SPI.h"

namespace Spi {
  constexpr int SER = 23;
  constexpr int SRCLK = 18;
  constexpr int RCLK = 19;

  SPIClass spi(HSPI);

  uint8_t data = 0;

  void sync() {
    digitalWrite(RCLK, LOW);
    spi.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE0));
    spi.transfer(data);
    spi.endTransaction();
    digitalWrite(RCLK, HIGH);
  }
  void setup() {
    spi.begin(SRCLK, -1, SER);
  }
}