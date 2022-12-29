#include <ClockLightSensor.h>

const ClockLightBrightness ClockLightSensor::brightnessLookupTable[] = {
    {0, 0, 4},
    {16, 4, 8},
    {32, 8, 12},
    {48, 12, 16},
    {64, 16, 20},
    {80, 20, 24},
    {96, 24, 28},
    {112, 28, 32},
    {128, 32, 36},
    {144, 36, 40},
    {160, 40, 44},
    {176, 44, 48},
    {192, 48, 52},
    {208, 52, 56},
    {224, 56, 60},
    {255, 60, 100000},
};

ClockLightSensor::ClockLightSensor()
{
    printk("Init Device VEML.\n");

    sensor = DEVICE_DT_GET(DT_INST(0, vishay_veml7700));
    
    if (!sensor) {
        printk("Device VEML not found.\n");
        return;
    }
}

uint32_t ClockLightSensor::getLightLux()
{
    struct sensor_value val;

    printk("Get lightLux.\n");

    if (!sensor) {
        return 0;
    }

    if (sensor_sample_fetch(sensor) < 0) {
        printk("sample update error.\n");
        return 0;
    } else {
        if (sensor_channel_get(sensor, SENSOR_CHAN_LIGHT, &val) < 0) {
            printk("ALS read error.\n");
            return 0;
        } else {
            printk("Light (lux)1: %d\n", (uint32_t)val.val1);
            return ((uint32_t)val.val1);
        }
    }
}

uint8_t ClockLightSensor::getBrightness()
{
    uint32_t luxValue = getLightLux();

    uint8_t brightness = 0;

    for (uint8_t i = 0; i < sizeof(brightnessLookupTable)/sizeof(brightnessLookupTable[0]); i++) {
        if ((luxValue >= brightnessLookupTable[i].from) && (luxValue <= brightnessLookupTable[i].to)) {
            brightness = brightnessLookupTable[i].brightness;
            break;
        }
    }

    printk("Display Brightness: %u\n", brightness);

    return brightness;
}
