/**
 * @file soldered_bmp180.c
 * @brief Implementation for the soldered-bmp180 component
 * @author Soldered Electronics
 */

#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soldered_bmp180.h"

/**
 * @brief Sleep for at least `ms` milliseconds
 *
 * pdMS_TO_TICKS() rounds down and the first tick can come at once, so at the
 * default 100 Hz tick a plain vTaskDelay(pdMS_TO_TICKS(5)) does not wait at all.
 */
static void bmp180_sleep_ms(uint32_t ms)
{
    vTaskDelay((ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS + 1);
}

static esp_err_t bmp180_read_regs(bmp180_t *dev, uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(dev->i2c_dev, &reg, 1, data, len, SOLDERED_BMP180_I2C_TIMEOUT_MS);
}

/**
 * @brief Start a conversion, wait for it and read `len` result bytes
 */
static esp_err_t bmp180_convert(bmp180_t *dev, uint8_t cmd, uint32_t wait_ms, uint8_t *data, size_t len)
{
    const uint8_t buf[2] = {BMP180_REG_CONTROL, cmd};
    esp_err_t err = i2c_master_transmit(dev->i2c_dev, buf, sizeof(buf), SOLDERED_BMP180_I2C_TIMEOUT_MS);

    if (err != ESP_OK) {
        return err;
    }
    bmp180_sleep_ms(wait_ms);

    return bmp180_read_regs(dev, BMP180_REG_RESULT, data, len);
}

esp_err_t soldered_bmp180_init(bmp180_t *dev, i2c_master_bus_handle_t bus, uint8_t oss)
{
    uint8_t raw[BMP180_CALIB_LEN];
    uint8_t chip_id;
    esp_err_t err;

    if ((dev == NULL) || (bus == NULL) || (oss > 3)) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(dev, 0, sizeof(*dev));
    dev->oss = oss;

    i2c_device_config_t i2c_conf = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BMP180_I2C_ADDR,
        .scl_speed_hz = SOLDERED_BMP180_SCL_SPEED_HZ,
    };

    err = i2c_master_bus_add_device(bus, &i2c_conf, &dev->i2c_dev);
    if (err != ESP_OK) {
        return err;
    }

    err = bmp180_read_regs(dev, BMP180_REG_CHIP_ID, &chip_id, 1);
    if ((err == ESP_OK) && (chip_id != BMP180_CHIP_ID)) {
        err = ESP_ERR_NOT_FOUND;
    }
    if (err == ESP_OK) {
        err = bmp180_read_regs(dev, BMP180_REG_CALIB, raw, sizeof(raw));
    }

    if (err != ESP_OK) {
        i2c_master_bus_rm_device(dev->i2c_dev);
        dev->i2c_dev = NULL;
        return err;
    }

    bmp180_parse_calib(raw, &dev->calib);

    return ESP_OK;
}

esp_err_t soldered_bmp180_deinit(bmp180_t *dev)
{
    esp_err_t err;

    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (dev->i2c_dev == NULL) {
        return ESP_OK;
    }

    err = i2c_master_bus_rm_device(dev->i2c_dev);
    dev->i2c_dev = NULL;

    return err;
}

esp_err_t soldered_bmp180_read(bmp180_t *dev, float *temperature, float *pressure)
{
    uint8_t ut[2], up[3];
    int32_t t, p;
    esp_err_t err;

    if ((dev == NULL) || (dev->i2c_dev == NULL) || (temperature == NULL) || (pressure == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }

    err = bmp180_convert(dev, BMP180_CMD_TEMPERATURE, BMP180_TEMPERATURE_WAIT_MS, ut, sizeof(ut));
    if (err == ESP_OK) {
        err = bmp180_convert(dev, BMP180_CMD_PRESSURE(dev->oss), BMP180_PRESSURE_WAIT_MS(dev->oss), up, sizeof(up));
    }
    if (err != ESP_OK) {
        return err;
    }

    bmp180_compensate(&dev->calib, (ut[0] << 8) | ut[1], ((up[0] << 16) | (up[1] << 8) | up[2]) >> (8 - dev->oss),
                      dev->oss, &t, &p);
    *temperature = t / 10.0f;
    *pressure = p / 100.0f;

    return ESP_OK;
}

float soldered_bmp180_sealevel(float pressure, float altitude)
{
    return pressure / powf(1.0f - altitude / 44330.0f, 5.255f);
}

float soldered_bmp180_altitude(float pressure, float baseline)
{
    return 44330.0f * (1.0f - powf(pressure / baseline, 1.0f / 5.255f));
}
