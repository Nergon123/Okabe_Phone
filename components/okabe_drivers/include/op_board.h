#pragma once
// Defaults mirror the existing board profiles; -D overrides remain supported.
#ifdef OKABE_S3
#ifndef TFT_MOSI
#define TFT_MOSI 41
#define TFT_MISO 39
#define TFT_SCLK 40
#define TFT_CS 1
#define TFT_DC 42
#define TFT_RST 2
#define TFT_BL -1
#define SPI_FREQUENCY 27000000
#endif
#ifndef I2C_SDA
#define I2C_SDA 8
#define I2C_SCL 9
#endif
#ifndef SIM_RX_PIN
#define SIM_RX_PIN 35
#define SIM_TX_PIN 26
#endif
#ifndef HAS_I2S
#define HAS_I2S
#define I2S_BCK 5
#define I2S_DIN 6
#define I2S_LRCK 7
#endif
#else
#ifndef TFT_MOSI
#define TFT_MOSI 23
#define TFT_MISO 12
#define TFT_SCLK 18
#define TFT_CS 27
#define TFT_DC 32
#define TFT_RST -1
#define TFT_BL 4
#define SPI_FREQUENCY 40000000
#endif
#ifndef I2C_SDA
#define I2C_SDA 21
#define I2C_SCL 22
#endif
#ifndef SIM_RX_PIN
#define SIM_RX_PIN -1
#define SIM_TX_PIN -1
#endif
#ifndef INV_DISP
#define INV_DISP
#endif
#endif
// No SD wiring was specified for the prototype. Enable only with all four pins.
#ifndef SD_CS
#define SD_CS -1
#define SD_SCK -1
#define SD_MISO -1
#define SD_MOSI -1
#endif

// Preserve the previous disabled UART until modem wiring is confirmed.
#ifndef OKABE_MODEM_ENABLED
#define OKABE_MODEM_ENABLED 0
#endif
