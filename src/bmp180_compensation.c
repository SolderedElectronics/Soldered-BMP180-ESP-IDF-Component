/**
 * @file bmp180_compensation.c
 * @brief BMP180 compensation math from the Bosch datasheet, section 3.5
 *
 * Divisions by powers of two are right shifts, as in the datasheet and
 * Bosch's reference driver; they round towards minus infinity, which the
 * datasheet's example values depend on.
 *
 * @author Soldered Electronics
 */

#include "bmp180_compensation.h"

static int16_t be16(const uint8_t *p)
{
    return (int16_t)((p[0] << 8) | p[1]);
}

void bmp180_parse_calib(const uint8_t raw[BMP180_CALIB_LEN], bmp180_calib_t *calib)
{
    calib->ac1 = be16(&raw[0]);
    calib->ac2 = be16(&raw[2]);
    calib->ac3 = be16(&raw[4]);
    calib->ac4 = (uint16_t)be16(&raw[6]);
    calib->ac5 = (uint16_t)be16(&raw[8]);
    calib->ac6 = (uint16_t)be16(&raw[10]);
    calib->b1 = be16(&raw[12]);
    calib->b2 = be16(&raw[14]);
    calib->mb = be16(&raw[16]);
    calib->mc = be16(&raw[18]);
    calib->md = be16(&raw[20]);
}

void bmp180_compensate(const bmp180_calib_t *calib, int32_t ut, int32_t up, uint8_t oss, int32_t *temperature,
                       int32_t *pressure)
{
    int32_t x1, x2, x3, b3, b5, b6, p;
    uint32_t b4, b7;

    x1 = ((ut - calib->ac6) * calib->ac5) >> 15;
    x2 = ((int32_t)calib->mc << 11) / (x1 + calib->md);
    b5 = x1 + x2;
    *temperature = (b5 + 8) >> 4;

    b6 = b5 - 4000;
    x1 = (calib->b2 * ((b6 * b6) >> 12)) >> 11;
    x2 = (calib->ac2 * b6) >> 11;
    x3 = x1 + x2;
    b3 = ((((int32_t)calib->ac1 * 4 + x3) << oss) + 2) >> 2;
    x1 = (calib->ac3 * b6) >> 13;
    x2 = (calib->b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (calib->ac4 * (uint32_t)(x3 + 32768)) >> 15;
    b7 = ((uint32_t)up - b3) * (50000 >> oss);
    p = (b7 < 0x80000000) ? (int32_t)((b7 * 2) / b4) : (int32_t)((b7 / b4) * 2);

    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    *pressure = p + ((x1 + x2 + 3791) >> 4);
}
