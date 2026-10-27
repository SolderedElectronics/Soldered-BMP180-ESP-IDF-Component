/**
 * @file soldered_bmp180.h
 * @brief Public API for the soldered-bmp180 component
 *
 * ESP-IDF driver for the Soldered BMP180 temperature and pressure sensor
 * breakout board over I2C. The register map and compensation math live in
 * bmp180_compensation.h.
 *
 * The I2C bus belongs to the application, not to this driver, so that other
 * Qwiic devices can share it. Create it with i2c_new_master_bus() and hand the
 * handle to soldered_bmp180_init().
 *
 * @author Soldered Electronics
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "bmp180_compensation.h"
#include "driver/i2c_master.h"
#include "esp_err.h"

/** I2C clock the sensor is driven at; the BMP180 supports up to 3.4 MHz, ESP32 masters up to 400 kHz */
#define SOLDERED_BMP180_SCL_SPEED_HZ 400000

/** How long a single I2C transaction may take before it is given up on */
#define SOLDERED_BMP180_I2C_TIMEOUT_MS 1000

/**
 * @brief Handle for one BMP180 breakout board
 *
 * All fields are managed by the driver.
 */
typedef struct {
    i2c_master_dev_handle_t i2c_dev; /**< I2C device handle, created by soldered_bmp180_init() */
    uint8_t oss;                     /**< Pressure oversampling setting, 0 to 3 */
    bmp180_calib_t calib;            /**< Factory calibration read at init */
} bmp180_t;

/**
 * @brief Attach a BMP180 to an already initialized I2C bus
 *
 * Adds the sensor as a device on `bus` (address 0x77), checks its chip ID and
 * reads its factory calibration.
 *
 * @param[out] dev Handle to initialize
 * @param[in] bus I2C bus the breakout is wired to, created with i2c_new_master_bus()
 * @param[in] oss Pressure oversampling setting: 0 (one sample, 5 ms) to 3 (eight samples, 26 ms).
 *                Higher settings are slower but less noisy
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG on a NULL argument or `oss` above 3,
 *         ESP_ERR_NOT_FOUND if the device on 0x77 is not a BMP180, or the I2C error
 */
esp_err_t soldered_bmp180_init(bmp180_t *dev, i2c_master_bus_handle_t bus, uint8_t oss);

/**
 * @brief Detach the sensor from the I2C bus
 *
 * Does not delete the bus itself, since the bus is owned by the caller.
 *
 * @param[in,out] dev Handle previously initialized with soldered_bmp180_init()
 *
 * @return ESP_OK on success, or the error returned by i2c_master_bus_rm_device()
 */
esp_err_t soldered_bmp180_deinit(bmp180_t *dev);

/**
 * @brief Measure temperature and pressure
 *
 * Runs a temperature conversion and then a pressure conversion, blocking the
 * calling task for about 10 ms plus the pressure conversion time.
 *
 * @param[in] dev Handle
 * @param[out] temperature Temperature in degrees Celsius
 * @param[out] pressure Absolute pressure in hPa (same as mbar)
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG on a NULL argument, or the I2C error
 */
esp_err_t soldered_bmp180_read(bmp180_t *dev, float *temperature, float *pressure);

/**
 * @brief Convert an absolute pressure to the equivalent pressure at sea level
 *
 * This is the value weather reports use.
 *
 * @param[in] pressure Absolute pressure in hPa, measured at `altitude`
 * @param[in] altitude Altitude of the measurement in meters
 *
 * @return Sea-level pressure in hPa
 */
float soldered_bmp180_sealevel(float pressure, float altitude);

/**
 * @brief Compute the altitude difference between two pressures
 *
 * @param[in] pressure Absolute pressure in hPa
 * @param[in] baseline Pressure at the reference point in hPa, e.g. the sea-level pressure
 *
 * @return Altitude above the reference point in meters
 */
float soldered_bmp180_altitude(float pressure, float baseline);

#ifdef __cplusplus
}
#endif
