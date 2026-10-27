/**
 * @file main.c
 * @brief Reads temperature and pressure from the BMP180 sensor over I2C
 *
 * Also converts the pressure to sea-level pressure and back to an altitude,
 * like the Arduino library's TempAndPressure example.
 *
 * Connect the breakout board to the I2C pins of your board, or use a Qwiic
 * cable.
 *
 * Product used is www.solde.red/333060
 *
 * @author Soldered Electronics
 */

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soldered_bmp180.h"

static const char *TAG = "BMP180_BASIC";

// Change these to match how your breakout is wired
#define PIN_NUM_SDA GPIO_NUM_21
#define PIN_NUM_SCL GPIO_NUM_22

// Altitude of the place the sensor is at, in meters (Osijek, Croatia)
#define ALTITUDE_M 90.0f

void app_main(void)
{
    bmp180_t sensor;
    float temperature, pressure;

    // The I2C bus belongs to the application, not to the driver, so that other
    // Qwiic devices can share it. Create it first, then hand it to the driver.
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_NUM_SDA,
        .scl_io_num = PIN_NUM_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    // Oversampling 3: slowest (26 ms per pressure reading) but least noisy
    esp_err_t err = soldered_bmp180_init(&sensor, bus, 3);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "BMP180 initialization failed: %s", esp_err_to_name(err));
        return;
    }

    while (1) {
        err = soldered_bmp180_read(&sensor, &temperature, &pressure);
        if (err == ESP_OK) {
            float sealevel = soldered_bmp180_sealevel(pressure, ALTITUDE_M);

            ESP_LOGI(TAG, "Temperature: %.1f degC, pressure: %.2f hPa", temperature, pressure);
            ESP_LOGI(TAG, "Sea-level pressure: %.2f hPa, computed altitude: %.0f m", sealevel,
                     soldered_bmp180_altitude(pressure, sealevel));
        } else {
            ESP_LOGE(TAG, "Reading the sensor failed: %s", esp_err_to_name(err));
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
