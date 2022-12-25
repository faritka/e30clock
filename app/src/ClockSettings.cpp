#include <ClockSettings.h>

ClockSettings::ClockSettings()
{
    setDefaultValues();

    load();

    readFormat();
}

void ClockSettings::setDefaultValues()
{
    printk("Setting the default values\n");

    //settings ID that verifies the data was loaded correctly
    this->settingsId = 0x4322;

    TimeChangeRule dstRule = {"PDT", Second, Sun, Mar, 2, -420};
    this->dstRule = dstRule;
    TimeChangeRule stdRule = {"PST", First, Sun, Nov, 2, -480};
    this->stdRule = stdRule;

    this->hourlyAlarm = true;

    //temperature in Celsius
    this->formatTemperature = ClockSettings::formatCelsius;
    //24-hour clock
    this->formatHour = ClockSettings::formatHour24;

}

const struct device *ClockSettings::getEepromDevice()
{
    printk("Init Device EEPROM.\n");

    const struct device *eeprom = device_get_binding(DT_LABEL(DT_INST(0, atmel_at24)));
    
    if (!device_is_ready(eeprom)) {
        printk("Device EEPROM not found.\n");
        return NULL;
    }

        printk("Device EEPROM was initialized.\n");

    return eeprom;
}

int ClockSettings::load()
{
    const struct device *eeprom = getEepromDevice();

    if (eeprom == NULL) {
        return 1;
    }

    int ret = eeprom_read(eeprom, 0, (void *)this, sizeof(*this));
    if (ret < 0) {
        printk("Error: Couldn't read eeprom: err: %d.\n", ret);
        return 1;
    }

    printk("settingsId: %zu\n", this->settingsId);

    printk("The config was read from EEPROM\n");    

    //the settings ID doesn't match, load the default values
    if (this->settingsId != 0x4322) {
        setDefaultValues();
    }

    return 0;
}

int ClockSettings::save()
{
    const struct device *eeprom = getEepromDevice();

    //write all variables of the settings class to the EEPROM
    //const char *p = (const char*)(const void*)this;

    if (eeprom == NULL) {
        return 1;
    }

    int ret = eeprom_write(eeprom, 0, (const void*)this, sizeof(*this));
    if (ret < 0) {
        printk("Error: Couldn't write eeprom: err:%d.\n", ret);
        return 1;
    }

    printk("Wrote the settings object into the EEPROM.\n");

    return 0;
}

void ClockSettings::readFormat()
{
    int ret = 0;

    printk("Settings readFormat\n");

    //first set HIGH to the enable format pin
    const struct gpio_dt_spec enableFormat = GPIO_DT_SPEC_GET(DT_PATH(bmw_clock), enable_format_gpios);

    if (!device_is_ready(enableFormat.port)) {
        printk("Error: GPIO enableFormat %s is not ready\n", enableFormat.port->name);
    } else {    
        //active when the level is high
        ret = gpio_pin_configure_dt(&enableFormat, GPIO_OUTPUT_HIGH);
        if (ret != 0) {
            printk("Error %d: failed to configure %s pin %d\n",
                ret, enableFormat.port->name, enableFormat.pin);
        }
    }

    ret = gpio_pin_set_dt(&enableFormat, 1);
    if (ret != 0) {
        printk("Error %d: pin set enableFormat %s pin %d\n",
            ret, enableFormat.port->name, enableFormat.pin);
    }

    k_msleep(10);

    const struct gpio_dt_spec celsius12 = GPIO_DT_SPEC_GET(DT_PATH(bmw_clock), celsius12_gpios);
    if (readFormatPin(&celsius12)) {
        this->formatTemperature = ClockSettings::formatCelsius;
        this->formatHour = ClockSettings::formatHour12;
    }

    const struct gpio_dt_spec celsius24 = GPIO_DT_SPEC_GET(DT_PATH(bmw_clock), celsius24_gpios);
    if (readFormatPin(&celsius24)) {
        this->formatTemperature = ClockSettings::formatCelsius;
        this->formatHour = ClockSettings::formatHour24;
    }

    const struct gpio_dt_spec fahrenheit12 = GPIO_DT_SPEC_GET(DT_PATH(bmw_clock), fahrenheit12_gpios);
    if (readFormatPin(&fahrenheit12)) {
        this->formatTemperature = ClockSettings::formatFahrenheit;
        this->formatHour = ClockSettings::formatHour12;
    }

    //disable the high level on the enable-format pin
    ret = gpio_pin_set_dt(&enableFormat, 0);
    if (ret != 0) {
        printk("Error %d: pin set enableFormat %s pin %d\n",
            ret, enableFormat.port->name, enableFormat.pin);
    }

    ret = gpio_pin_configure_dt(&enableFormat, GPIO_INPUT);
}

uint8_t ClockSettings::readFormatPin(const struct gpio_dt_spec *gpio)
{
    int ret = 0;
    int level = 0;

    printk("Configure %s pin %d\n",
                 gpio->port->name, gpio->pin);

    if (!device_is_ready(gpio->port)) {
        printk("Error: GPIO %s is not ready\n", gpio->port->name);
    } else {    
        ret = gpio_pin_configure_dt(gpio, GPIO_INPUT | GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH);
        if (ret != 0) {
            printk("Error %d: failed to configure %s pin %d\n",
                ret, gpio->port->name, gpio->pin);
        }

        k_msleep(10);

        level = gpio_pin_get_dt(gpio);
        printk("\nLevel: %d\n", level);
    }

    return level;
}

