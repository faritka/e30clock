/*
 * Copyright (c) 2021 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_SENSOR_VEML7700_H_
#define ZEPHYR_DRIVERS_SENSOR_VEML7700_H_

#include <drivers/sensor.h>
#include <drivers/i2c.h>
#include <pm/device.h>

// The slave address (7 bit) is set to 0010000 = 0x10.
// The least significant bit (LSB) defines read or write mode.
// Accordingly, for 8 bit the bus address is then 0010 0000 = 20h for write and 0010 0001 = 21h for read
#define VEML7700_I2CADDR 0x10

// Configuration Register
#define VEML7700_REG_CONF 0x00
// ALS High Resolution Output Data register
#define VEML7700_REG_ALS_DATA 0x04
// Power Saving Mode register
#define VEML7700_REG_PSM 0x03

// The position of ALS gain in the config register
#define VEML7700_ALS_GAIN_POS 11

// The position of ALS integration time in the config register
#define VEML7700_ALS_IT_POS 6

// The position of ALS persistence protect number 
#define VEML7700_ALS_PERS_POS 4

// ALS shutdown bit is 0
#define VEML7700_ALS_SD_MASK BIT(0)

struct veml7700_config {
    char *i2c_name;
    uint8_t i2c_address;
    // Ambient light sensor gain selection
    uint8_t als_gain;
    // ALS integration time setting
    uint8_t als_it;
    // ALS persistence protect number setting
    uint8_t als_pers;
    // Power saving mode
    uint8_t psm;
};

struct veml7700_data {
    const struct device *i2c;
    struct k_sem sem;
    uint16_t light;
};

int veml7700_read(const struct device *dev, uint8_t reg, uint16_t *out);
int veml7700_write(const struct device *dev, uint8_t reg, uint16_t value);

#endif
