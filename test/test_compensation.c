/*
 * Host-side check of the compensation math against the worked example in the
 * BMP180 datasheet (BST-BMP180-DS000, section 3.5). No ESP-IDF needed:
 *
 *   cc -std=c99 -Wall -Wextra -Iinclude src/bmp180_compensation.c test/test_compensation.c -o /tmp/t && /tmp/t
 */

#include <assert.h>
#include <stdio.h>
#include "bmp180_compensation.h"

int main(void)
{
    // AC1=408 AC2=-72 AC3=-14383 AC4=32741 AC5=32757 AC6=23153 B1=6190 B2=4 MB=-32768 MC=-8711 MD=2868
    const uint8_t raw[BMP180_CALIB_LEN] = {0x01, 0x98, 0xFF, 0xB8, 0xC7, 0xD1, 0x7F, 0xE5, 0x7F, 0xF5, 0x5A,
                                           0x71, 0x18, 0x2E, 0x00, 0x04, 0x80, 0x00, 0xDD, 0xF9, 0x0B, 0x34
                                          };
    bmp180_calib_t calib;
    int32_t temperature, pressure;

    bmp180_parse_calib(raw, &calib);
    assert(calib.ac1 == 408 && calib.ac2 == -72 && calib.ac3 == -14383);
    assert(calib.ac4 == 32741 && calib.ac5 == 32757 && calib.ac6 == 23153);
    assert(calib.b1 == 6190 && calib.b2 == 4 && calib.mb == -32768 && calib.mc == -8711 && calib.md == 2868);

    bmp180_compensate(&calib, 27898, 23843, 0, &temperature, &pressure);
    printf("temperature=%ld (0.1 degC) pressure=%ld Pa\n", (long)temperature, (long)pressure);
    assert(temperature == 150);
    assert(pressure == 69964);

    assert(BMP180_CMD_PRESSURE(3) == 0xF4);
    assert(BMP180_PRESSURE_WAIT_MS(0) == 5 && BMP180_PRESSURE_WAIT_MS(3) == 26);

    puts("OK");
    return 0;
}
