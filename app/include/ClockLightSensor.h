/*
 * The class for the light sensor
 * 
 */
#ifndef __CLOCK_LIGHT_SENSOR_H
#define __CLOCK_LIGHT_SENSOR_H

#include <zephyr/sys/printk.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>

struct ClockLightBrightness {
    //from 0 to 255
    uint8_t brightness;
    //from the light value in Lux
    uint32_t from;
    //to the light value in Lux
    uint32_t to;
};

class ClockLightSensor
{
    public:
        ClockLightSensor();

        //get the light value in luxes
        uint32_t getLightLux();

        //get the light brightness using the lookup table
        uint8_t getBrightness();

    private:
        //the light sensor device
        const struct device *sensor;

        //the Lux light value to the brightness(0-255) lookup table
        const static ClockLightBrightness brightnessLookupTable[16];

};

#endif

