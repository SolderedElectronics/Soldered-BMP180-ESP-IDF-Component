/**
 * @file bmp180_compensation.h
 * @brief BMP180 register map and compensation math, with no platform dependencies
 *
 * Integer compensation algorithm from the Bosch BMP180 datasheet
 * (BST-BMP180-DS000, section 3.5). The same file is used unchanged by the
 * Soldered BMP180 ESPHome component.
 *
 * @author Soldered Electronics
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** The BMP180 has a single, fixed I2C address */
#define BMP180_I2C_ADDR 0x77

#define BMP180_REG_CALIB 0xAA   /**< First of the 22 calibration bytes */
#define BMP180_REG_CHIP_ID 0xD0 /**< Reads ::BMP180_CHIP_ID */
#define BMP180_REG_CONTROL 0xF4 /**< Measurement command register */
#define BMP180_REG_RESULT 0xF6  /**< Measurement result, MSB first */

#define BMP180_CHIP_ID 0x55
#define BMP180_CALIB_LEN 22

/** Starts a temperature conversion, result ready after ::BMP180_TEMPERATURE_WAIT_MS */
#define BMP180_CMD_TEMPERATURE 0x2E
#define BMP180_TEMPERATURE_WAIT_MS 5

/** Starts a pressure conversion at oversampling setting `oss` (0 to 3) */
#define BMP180_CMD_PRESSURE(oss) (0x34 + ((oss) << 6))

/** Conversion time of a pressure measurement at oversampling setting `oss`: 5, 8, 14 or 26 ms (datasheet max
 * 4.5, 7.5, 13.5 and 25.5 ms, rounded up) */
#define BMP180_PRESSURE_WAIT_MS(oss) (2 + (3 << (oss)))

/**
 * @brief Factory calibration coefficients, stored in each sensor's EEPROM
 */
typedef struct {
    int16_t ac1, ac2, ac3;
    uint16_t ac4, ac5, ac6;
    int16_t b1, b2, mb, mc, md;
} bmp180_calib_t;

/**
 * @brief Decode the ::BMP180_CALIB_LEN bytes read from ::BMP180_REG_CALIB
 *
 * @param[in] raw Calibration bytes as read from the sensor
 * @param[out] calib Decoded coefficients
 */
void bmp180_parse_calib(const uint8_t raw[BMP180_CALIB_LEN], bmp180_calib_t *calib);

/**
 * @brief Turn raw readings into temperature and pressure
 *
 * @param[in] calib Calibration coefficients of this sensor
 * @param[in] ut Raw temperature, the 16-bit result of a temperature conversion
 * @param[in] up Raw pressure, the 24-bit result of a pressure conversion shifted right by (8 - oss)
 * @param[in] oss Oversampling setting the pressure conversion ran at, 0 to 3
 * @param[out] temperature Temperature in 0.1 degrees Celsius
 * @param[out] pressure Pressure in Pa
 */
void bmp180_compensate(const bmp180_calib_t *calib, int32_t ut, int32_t up, uint8_t oss, int32_t *temperature,
                       int32_t *pressure);

#ifdef __cplusplus
}
#endif
