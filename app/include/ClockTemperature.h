/*
 * The class for the temperature sensor
 * 
 */
#ifndef __CLOCK_TEMPERATURE_H
#define __CLOCK_TEMPERATURE_H

#include <zephyr/sys/printk.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>

#include <stdlib.h>

struct ClockNtc {
    //T [°C] 
    int temperature;
    //R nom [Ω]
    float resistanceNom;
    //R min [Ω]
    float resistanceMin;
    //R max [Ω] 
    float resistanceMax;
};

class ClockTemperature
{
    public:
        ClockTemperature();

        //get the temperature in degrees Celcius
        int getTemperature();

    private:

        //the ADC device
        const struct device *adcDevice;

        struct adc_sequence *adcSequence;

        uint8_t channelId;

        //enable the pin before running the measurements
        struct gpio_dt_spec enablePin;

        //the resistance to temperature lookup table for the NTC resistor
        const static ClockNtc ntcLookupTable[108];

        //the analog reference voltage for ADC
        const static uint16_t adcVref = 3300;

        //the resistor divider top part resistor value in Ohm
        const static uint16_t dividerResistor = 4700;

        //the mutex to limit simultaneous access to the ADC hardware
        struct k_mutex mutexAdc;

};

#endif

