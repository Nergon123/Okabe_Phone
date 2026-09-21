#include "boot_i2c.h"

#include <stdbool.h>
#include <stdint.h>

#include "esp_rom_gpio.h"
#include "esp_rom_sys.h"
#include "soc/gpio_periph.h"
#include "soc/gpio_struct.h"
#include "soc/io_mux_reg.h"

#ifndef I2C_SDA
#define I2C_SDA 14
#endif

#ifndef I2C_SCL
#define I2C_SCL 13
#endif
#define I2C_DELAY_US      5
#define I2C_CLOCK_TIMEOUT 1000


static inline void i2c_delay(void)
{
    esp_rom_delay_us(I2C_DELAY_US);
}


/*
 * I2C is open-drain:
 *
 * LOW  = enable output, output value = 0
 * HIGH = disable output and let pull-up raise the line
 *
 * Never actively drive SDA/SCL high.
 */

static inline void sda_low(void)
{
    GPIO.out_w1tc = (1U << I2C_SDA);
    GPIO.enable_w1ts = (1U << I2C_SDA);
}

static inline void sda_release(void)
{
    GPIO.enable_w1tc = (1U << I2C_SDA);
}

static inline void scl_low(void)
{
    GPIO.out_w1tc = (1U << I2C_SCL);
    GPIO.enable_w1ts = (1U << I2C_SCL);
}

static inline void scl_release(void)
{
    GPIO.enable_w1tc = (1U << I2C_SCL);
}

static inline bool sda_read(void)
{
    return (GPIO.in >> I2C_SDA) & 1U;
}

static inline bool scl_read(void)
{
    return (GPIO.in >> I2C_SCL) & 1U;
}


static bool wait_scl_high(void)
{
    scl_release();

    for (unsigned i = 0; i < I2C_CLOCK_TIMEOUT; ++i) {
        if (scl_read()) {
            return true;
        }

        esp_rom_delay_us(1);
    }

    return false;
}

void boot_i2c_init(void)
{
    /*
     * Route pads to GPIO.
     */
    esp_rom_gpio_pad_select_gpio(I2C_SDA);
    esp_rom_gpio_pad_select_gpio(I2C_SCL);

    /*
     * Enable input path.
     *
     * Necessary because we need to read SDA for ACK/data
     * and SCL for clock-stretching.
     */
    PIN_INPUT_ENABLE(GPIO_PIN_MUX_REG[I2C_SDA]);
    PIN_INPUT_ENABLE(GPIO_PIN_MUX_REG[I2C_SCL]);

    /*
     * Internal pull-ups are useful during boot.
     * External I2C pull-ups should still exist on the board.
     */
    esp_rom_gpio_pad_pullup_only(I2C_SDA);
    esp_rom_gpio_pad_pullup_only(I2C_SCL);

    /*
     * Output register permanently contains LOW.
     *
     * I2C "HIGH" is implemented by disabling the output,
     * allowing the pull-up to raise the line.
     */
    GPIO.out_w1tc =
        (1U << I2C_SDA) |
        (1U << I2C_SCL);

    /*
     * Release both lines.
     */
    GPIO.enable_w1tc =
        (1U << I2C_SDA) |
        (1U << I2C_SCL);

    esp_rom_delay_us(I2C_DELAY_US);
}

static esp_err_t i2c_start(void)
{
    /*
     * START:
     *
     * SDA ──────┐
     *           └────
     *
     * SCL ───────────
     */

    sda_release();

    if (!wait_scl_high()) {
        return ESP_ERR_TIMEOUT;
    }

    i2c_delay();

    if (!sda_read()) {
        // Bus appears busy.
        return ESP_ERR_INVALID_STATE;
    }

    sda_low();
    i2c_delay();

    scl_low();
    i2c_delay();

    return ESP_OK;
}


static esp_err_t i2c_restart(void)
{
    /*
     * Generate repeated START.
     */

    sda_release();
    i2c_delay();

    if (!wait_scl_high()) {
        return ESP_ERR_TIMEOUT;
    }

    i2c_delay();

    sda_low();
    i2c_delay();

    scl_low();
    i2c_delay();

    return ESP_OK;
}


static esp_err_t i2c_stop(void)
{
    /*
     * STOP:
     *
     * SDA ____/
     *
     * SCL ─────────
     */

    sda_low();
    i2c_delay();

    if (!wait_scl_high()) {
        sda_release();
        return ESP_ERR_TIMEOUT;
    }

    i2c_delay();

    sda_release();
    i2c_delay();

    return ESP_OK;
}


static esp_err_t i2c_write_byte(uint8_t value)
{
    for (int bit = 7; bit >= 0; --bit) {

        if (value & (1U << bit)) {
            sda_release();
        } else {
            sda_low();
        }

        i2c_delay();

        if (!wait_scl_high()) {
            return ESP_ERR_TIMEOUT;
        }

        i2c_delay();

        scl_low();
        i2c_delay();
    }

    /*
     * ACK cycle.
     *
     * Slave pulls SDA LOW = ACK.
     */
    sda_release();
    i2c_delay();

    if (!wait_scl_high()) {
        return ESP_ERR_TIMEOUT;
    }

    i2c_delay();

    bool ack = !sda_read();

    scl_low();
    i2c_delay();

    return ack ? ESP_OK : ESP_ERR_NOT_FOUND;
}


static esp_err_t i2c_read_byte(uint8_t *value, bool ack)
{
    if (!value) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t result = 0;

    sda_release();

    for (int bit = 7; bit >= 0; --bit) {

        i2c_delay();

        if (!wait_scl_high()) {
            return ESP_ERR_TIMEOUT;
        }

        i2c_delay();

        if (sda_read()) {
            result |= (1U << bit);
        }

        scl_low();
        i2c_delay();
    }

    /*
     * Master ACK/NACK.
     *
     * ACK  = SDA LOW
     * NACK = SDA released HIGH
     */

    if (ack) {
        sda_low();
    } else {
        sda_release();
    }

    i2c_delay();

    if (!wait_scl_high()) {
        sda_release();
        return ESP_ERR_TIMEOUT;
    }

    i2c_delay();

    scl_low();
    i2c_delay();

    sda_release();

    *value = result;

    return ESP_OK;
}


esp_err_t boot_i2c_write_reg(
    uint8_t address,
    uint8_t reg,
    uint8_t value)
{
    esp_err_t err;

    err = i2c_start();
    if (err != ESP_OK) {
        return err;
    }

    // 7-bit address + WRITE
    err = i2c_write_byte((address << 1) | 0);

    if (err == ESP_OK) {
        err = i2c_write_byte(reg);
    }

    if (err == ESP_OK) {
        err = i2c_write_byte(value);
    }

    esp_err_t stop_err = i2c_stop();

    if (err != ESP_OK) {
        return err;
    }

    return stop_err;
}


esp_err_t boot_i2c_read_reg(
    uint8_t address,
    uint8_t reg,
    uint8_t *data,
    size_t len)
{
    if (!data || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err;

    err = i2c_start();
    if (err != ESP_OK) {
        return err;
    }

    /*
     * First transaction:
     *
     * START
     * addr + W
     * register
     */

    err = i2c_write_byte((address << 1) | 0);

    if (err == ESP_OK) {
        err = i2c_write_byte(reg);
    }

    /*
     * Repeated START:
     *
     * addr + R
     */

    if (err == ESP_OK) {
        err = i2c_restart();
    }

    if (err == ESP_OK) {
        err = i2c_write_byte((address << 1) | 1);
    }

    /*
     * ACK every byte except the final one.
     */

    if (err == ESP_OK) {
        for (size_t i = 0; i < len; ++i) {

            bool ack = (i + 1) < len;

            err = i2c_read_byte(&data[i], ack);

            if (err != ESP_OK) {
                break;
            }
        }
    }

    esp_err_t stop_err = i2c_stop();

    if (err != ESP_OK) {
        return err;
    }

    return stop_err;
}