#include "boot_mcp23017.h"
#include "boot_i2c.h"

#include "esp_rom_sys.h"

#define MCP23017_ADDR 0x20

#define IODIRA  0x00
#define IODIRB  0x01

#define GPIOA   0x12
#define GPIOB   0x13

#define OLATA   0x14
#define OLATB   0x15

static esp_err_t mcp_write(uint8_t reg, uint8_t value)
{
    return boot_i2c_write_reg(
        MCP23017_ADDR,
        reg,
        value
    );
}

static esp_err_t mcp_read(uint8_t reg, uint8_t *value)
{
    return boot_i2c_read_reg(
        MCP23017_ADDR,
        reg,
        value,
        1
    );
}

esp_err_t boot_mcp23017_recovery_pressed(bool *pressed)
{
    if (!pressed)
        return ESP_ERR_INVALID_ARG;

    *pressed = false;

    esp_err_t err;
    uint8_t cols;

    /*
     * Columns on port B.
     */
    err = mcp_write(IODIRB, 0xFF);
    if (err != ESP_OK)
        return err;

    /*
     * Row 5 = GPA5.
     *
     * Set row 5 LOW, everything else HIGH.
     */
    err = mcp_write(OLATA, ~(1U << 5));
    if (err != ESP_OK)
        return err;

    /*
     * Drive port A.
     */
    err = mcp_write(IODIRA, 0x00);
    if (err != ESP_OK)
        return err;

    esp_rom_delay_us(100);

    err = mcp_read(GPIOB, &cols);

    /*
     * Release rows again.
     */
    mcp_write(IODIRA, 0xFF);

    if (err != ESP_OK)
        return err;

    /*
     * Active-low keyboard columns.
     */
    cols = (~cols) & 0x07;

    /*
     * Key 7 = column 1
     * Key 9 = column 3
     */
    *pressed =
        (cols & (1U << 0)) &&
        (cols & (1U << 2));

    return ESP_OK;
}