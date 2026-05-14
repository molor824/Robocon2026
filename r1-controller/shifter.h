#pragma once

#include <SPI.h>

namespace Shifter {
  constexpr int SCLK = 18, MOSI = 23, CS = 5;
  constexpr int CLOCK = 10000000;

  SPIClass *spi = 0;
  uint16_t data, _data;

  void setup() {
    spi = new SPIClass(HSPI);

    spi->begin(SCLK, -1, MOSI, -1);
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);
  }

  void sync() {
    if (data == _data) return;
    _data = data;

    Serial.printf("Shifting: %x\n", data);

    digitalWrite(CS, LOW);
    spi->beginTransaction(SPISettings(CLOCK, MSBFIRST, SPI_MODE0));
    spi->transfer16(data);
    spi->endTransaction();
    digitalWrite(CS, HIGH);
  }
}
