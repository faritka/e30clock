#include <ClockBackgroundLight.h>

gpio_callback ClockBackgroundLight::callbackData;

struct k_sem ClockBackgroundLight::interruptSemaphore;

ClockBackgroundLight::ClockBackgroundLight(ClockDisplay *clockDisplay)
{
    struct display_capabilities capabilities;

    this->clockDisplay = clockDisplay;

    if (clockDisplay->display == NULL) {
        printk("Device HT1632C not found in Background Light info.\n");
        return;
    }

    k_sem_init(&interruptSemaphore, 0, 1);

    display_get_capabilities(clockDisplay->display, &capabilities);

    //32 rows of 8-bit or 24 rows of 16-bit
    bufDesc.buf_size = displayWidth * capabilities.y_resolution;

    bufDesc.height = capabilities.y_resolution;
    bufDesc.pitch = 1;
    bufDesc.width = displayWidth;

    initInterrupt();
}


void ClockBackgroundLight::render()
{
    printk("backgroundLightInterruptCalled was On\n");

    const struct gpio_dt_spec lightSensor = getSensor();

    int level = gpio_pin_get_dt(&lightSensor);

    printk("Background Light Input Level: %d\n", level);

    lightOn = (level ? true : false);

    set(lightOn);

    //wait for milleseconds until the display is free
    //if not, if can display the time later
    if (k_mutex_lock(&clockDisplay->mutexDisplay, K_MSEC(300)) == 0) {
        //send the rows 30 and 31 starting from the 3CH address
        display_write(clockDisplay->display, 0x3c, 0, &bufDesc, bufDisplay);

        k_mutex_unlock(&clockDisplay->mutexDisplay);
    } else {
        printk("Display Mutex timeout\n");
    }

}


/**
 * Sets the background light for the buttons
 *
 * @param bool onOff If true, turn it on
 */
void ClockBackgroundLight::set(bool onOff)
{
    //Sets the ROW30 and ROW31 and COL7
    if (onOff == true) {
        bufDisplay[0] = bufDisplay[1] = 0x01;
    } else {
        bufDisplay[0] = bufDisplay[1] = 0x00;
    }
}

/**
 * Gets the background light sensor
 */
const struct gpio_dt_spec ClockBackgroundLight::getSensor()
{
    const struct gpio_dt_spec lightSensor = GPIO_DT_SPEC_GET(DT_PATH(bmw_clock), background_gpios);

    return lightSensor;
}

/**
 * Inits the background light sensor interrupt
 */
int ClockBackgroundLight::initInterrupt()
{
    const struct gpio_dt_spec gpio = getSensor();

    if (!device_is_ready(gpio.port)) {
        printk("Error: GPIO light device %s is not ready\n", gpio.port->name);
        return 1;
    }

    //active when the level is low
    int ret = gpio_pin_configure_dt(&gpio, GPIO_INPUT | GPIO_ACTIVE_LOW | GPIO_PULL_UP);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n",
           ret, gpio.port->name, gpio.pin);
        return 1;
    }

    ret = gpio_pin_interrupt_configure_dt(&gpio, GPIO_INT_EDGE_BOTH);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n",
            ret, gpio.port->name, gpio.pin);
        return 1;
    }

    gpio_init_callback(&callbackData, changed, BIT(gpio.pin));
    gpio_add_callback(gpio.port, &callbackData);

    printk("Set up background light GPIO at %s pin %d\n", gpio.port->name, gpio.pin);

    uint8_t levelRaw = gpio_pin_get_raw(gpio.port, gpio.pin);
    printk("\nLevelRaw: %u\n", levelRaw);

    int level = gpio_pin_get_dt(&gpio);
    printk("\nLevel: %d\n", level);

    //check the background light sensor level the first time
    render();

    return 0;
}

void ClockBackgroundLight::changed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    printk("Background light GPIO interrupt: %s, pins: %zu\n", dev->name, pins);

    //send the semaphore signal to the ClockBackgroundLight thread
    k_sem_give(&interruptSemaphore);
}

void ClockBackgroundLight::process()
{
    printk("Process the ClockBackgroundLight thread.\n");

    while (1) {
        k_sem_take(&interruptSemaphore, K_FOREVER);

        printk("Processing in ClockBackgroundLight thread\n");

        render();

        k_sem_reset(&interruptSemaphore);

        printk("End processing in ClockbackgroundLight\n");

    }

}