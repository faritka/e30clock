/*
 * Copyright (c) 2021 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#define DT_DRV_COMPAT holtek_ht1632c

#include "ht1632c.h"

LOG_MODULE_REGISTER(display_ht1632c, CONFIG_DISPLAY_LOG_LEVEL);

/**
 * @brief Converts nanoseconds to the number of the processor system cycles
 *
 * @param uint32_t ns Nanoseconds
 *
 */
static inline uint32_t ht1632c_ns_to_sys_clock_hw_cycles(uint32_t ns)
{
    return ((uint64_t)sys_clock_hw_cycles_per_sec() * (ns) / NSEC_PER_SEC + 1);
}

/**
 * @brief Delays the execution waiting for processor cycles
 *
 * @param dev Pointer to device data
 * @param uint32_t cycles_to_wait How many processor cycles to wait
 *
 */
static void ht1632c_delay(uint32_t cycles_to_wait)
{
    uint32_t start = k_cycle_get_32();

    // Wait until the given number of cycles have passed
    while (k_cycle_get_32() - start < cycles_to_wait) {
    }
}

/**
 * @brief Sets the CS GPIO pin
 *
 * @param dev Pointer to the device config

 * @param int state 1 sets the active state, 0 - inactive (CS is active LOW)
 *
 */
static void ht1632c_set_cs_pin(const struct device *dev, int state)
{
    const struct ht1632c_config *config = (struct ht1632c_config *)dev->config;

    gpio_pin_set_dt(&config->cs_gpio, state);
}

/**
 * @brief Sets the WR GPIO pin
 *
 * @param dev Pointer to the device config
 * @param int state 1 sets the active state, 0 - inactive (WR is active LOW)
 *
 */
static void ht1632c_set_wr_pin(const struct device *dev, int state)
{
    const struct ht1632c_config *config = (struct ht1632c_config *)dev->config;

    gpio_pin_set_dt(&config->wr_gpio, state);
}

/**
 * @brief Sets the DATA GPIO pin
 *
 * @param dev Pointer to the device config
 * @param int state 1 sets the active state, 0 - inactive (DATA is active HIGH)
 *
 */
static void ht1632c_set_data_pin(const struct device *dev, int state)
{
    const struct ht1632c_config *config = (struct ht1632c_config *)dev->config;

    gpio_pin_set_dt(&config->data_gpio, state);
}

/**
 * @brief Writes bits to HT1632C
 *
 * @param dev Pointer to device config
 * @param uint16_t bits Contains the data bits
 * @param uint`6_t bits The first bit from which to write. (If 1, only the last bit will be written. If 8, 8 bits will be written.)
 *
 */
static void ht1632c_write_bits(const struct device *dev, 
        uint16_t bits, 
        uint16_t firstbit) 
{
    struct ht1632c_data *data = (struct ht1632c_data *)dev->data;

    bool state = false;

    while(firstbit) {
        if (bits & firstbit) {
            state = true;
        } else {
            state = false;
        }

        //set the WR pin low
        ht1632c_set_wr_pin(dev, true);
        //set the next DATA bit
        ht1632c_set_data_pin(dev, state);
        //wait for the half-clock cycle
        ht1632c_delay(data->delays->su);
        //the next DATA bit is read on WR going from LOW to HIGH
        ht1632c_set_wr_pin(dev, false);
        //wait the next half-clock cycle
        ht1632c_delay(data->delays->clk);

        firstbit >>= 1;
    }
}

/**
 * @brief Writes a command to HT1632C
 *
 * @param dev Pointer to device
 * @param uint8_t command Command without the first 3 bits 100
 *
 */
static void ht1632c_write_command(const struct device *dev, 
        uint8_t command)
{
    struct ht1632c_data *data = (struct ht1632c_data *)dev->data;

    //CS down
    ht1632c_delay(data->delays->cs);
    ht1632c_set_cs_pin(dev, true);
    ht1632c_delay(data->delays->su1);
    
    //100 - command mode
    ht1632c_write_bits(dev, HT1632C_COMMAND_HEADER, BIT(2));
    //the command itself
    ht1632c_write_bits(dev, command, BIT(7));
    //one extra bit
    ht1632c_write_bits(dev, 0, BIT(0));

    //set the DATA pin low waiting for the next command
    ht1632c_set_data_pin(dev, false);

    //CS UP
    ht1632c_delay(data->delays->h1);
    ht1632c_set_cs_pin(dev, false);
}

/**
 * @brief Write data to display
 *
 * @param dev Pointer to device structure
 * @param x x Coordinate of the upper left corner where to write the buffer.
 * @param y y Coordinate of the upper left corner where to write the buffer. Always 0.
 * @param desc Pointer to a structure describing the buffer layout
 * @param buf Pointer to buffer array
 *
 * @retval 0 on success else negative errno code.
 */
static int ht1632c_write(const struct device *dev,
            const uint16_t x,
            const uint16_t y,
            const struct display_buffer_descriptor *desc,
            const void *buf)
{
    struct ht1632c_data *data = (struct ht1632c_data *)dev->data;
    const uint8_t *write_buf8 = (uint8_t *)buf;
    const uint16_t *write_buf16 = (uint16_t *)buf;

    //Allowed configurations
    //32 ROW x 8 COM
    //or 24 ROW x 16 COM 

    __ASSERT((data->width == 24) || (data->width == 32), "The screen width can be 24 or 32 only");
    __ASSERT(buf != NULL, "Display buffer is not available");
    __ASSERT(y == 0, "Y-coordinate has to be 0");

    //CS down
    ht1632c_delay(data->delays->cs);
    ht1632c_set_cs_pin(dev, true);
    ht1632c_delay(data->delays->su1);
    
    //Writing data in the Successive Address Writing Mode
    //101-A6A5A4A3A2A1A0-D0D1D2D3-D0D1D2D3...
    //from the first RAM address to the last
    //101-0x00-D0D1D2D3-D0D1D2D3D4...

    //101 - write data mode
    ht1632c_write_bits(dev, HT1632C_DATA_HEADER, BIT(2));

    //start address - 0x00 if the start of the buffer
    ht1632c_write_bits(dev, x, BIT(6));

    for (int i = 0; i < desc->width; i++) {
        if (data->width == 32) {
            ht1632c_write_bits(dev, write_buf8[i], BIT(7));
        } else if (data->width == 24) {
            ht1632c_write_bits(dev, write_buf16[i], BIT(15));
        }
    }

    ht1632c_set_data_pin(dev, false);

    //CS UP
    ht1632c_delay(data->delays->h1);
    ht1632c_set_cs_pin(dev, false);

    return 0;
}

/**
 * @brief Read data from display
 *
 * @param dev Pointer to device structure
 * @param x x Coordinate of the upper left corner where to read from
 * @param y y Coordinate of the upper left corner where to read from
 * @param desc Pointer to a structure describing the buffer layout
 * @param buf Pointer to buffer array
 *
 * @retval 0 on success else negative errno code.
 */
static int ht1632c_read(const struct device *dev, const uint16_t x,
      const uint16_t y,
      const struct display_buffer_descriptor *desc,
      void *buf)
{
    return -ENOTSUP;
}

static void *ht1632c_get_framebuffer(const struct device *dev)
{
    return NULL;
}

/**
 * Turns on the diplay LEDs
  * @param dev Pointer to device structure
 */
static int ht1632c_blanking_off(const struct device *dev)
{
    ht1632c_write_command(dev, HT1632_LED_ON);

    return 0;
}

/**
 * Turns off the diplay LEDs
 * @param dev Pointer to device structure
 */
static int ht1632c_blanking_on(const struct device *dev)
{
    ht1632c_write_command(dev, HT1632_LED_OFF);

    return 0;
}

/**
 * Set the brightness of the display in steps of 1/256, where 255 is full
 * brightness and 0 is minimal.
 *
 * @param dev Pointer to device structure
 * @param contrast Contrast in steps of 1/256
 *
 * @retval 0 on success else negative errno code.
 */
static int ht1632c_set_brightness(const struct device *dev,
                const uint8_t brightness)
{
    //The PWM level for HT1632C must be 0-15
    uint8_t pwm_level;    
    pwm_level = brightness / 16;

    if (pwm_level > 15) {
        pwm_level = 15;
    }

    ht1632c_write_command(dev, HT1632_PWM + pwm_level);

    return 0;
}

static int ht1632c_set_contrast(const struct device *dev,
                const uint8_t contrast)
{
    return -ENOTSUP;
}

/**
 * @brief Get display capabilities
 *
 * @param dev Pointer to device structure
 * @param capabilities Pointer to capabilities structure to populate
 */
static void ht1632c_get_capabilities(const struct device *dev,
        struct display_capabilities *capabilities)
{
    struct ht1632c_data *data = (struct ht1632c_data *)dev->data;

    memset(capabilities, 0, sizeof(struct display_capabilities));
    capabilities->x_resolution = data->width;
    capabilities->y_resolution = data->height;

    capabilities->supported_pixel_formats = PIXEL_FORMAT_MONO01;
    capabilities->current_pixel_format = PIXEL_FORMAT_MONO01;
    capabilities->screen_info = SCREEN_INFO_X_ALIGNMENT_WIDTH;
    capabilities->current_orientation = DISPLAY_ORIENTATION_NORMAL;
}

/**
 * @brief Set pixel format used by the display
 *
 * @param dev Pointer to device structure
 * @param pixel_format Pixel format to be used by display
 *
 * @retval 0 on success else negative errno code.
 */
static int ht1632c_set_pixel_format(const struct device *dev,
        const enum display_pixel_format pixel_format)
{
    return -ENOTSUP;
}

#ifdef CONFIG_PM_DEVICE
/**
 * Turns on the chip from sleep
 * @param dev Pointer to data structure
 */
static int ht1632c_exit_sleep(const struct device *dev)
{
    ht1632c_write_command(dev, HT1632_SYS_ON);
    ht1632c_write_command(dev, HT1632_LED_ON);

    k_sleep(K_MSEC(120));

    return 0;
}

/**
 * Turns off the chip into sleep
 * @param dev Pointer to data structure
 */
static int ht1632c_enter_sleep(const struct device *dev)
{
    ht1632c_write_command(dev, HT1632_SYS_DIS);
    ht1632c_write_command(dev, HT1632_LED_OFF);

    return 0;
}

static int ht1632c_pm_action(const struct device *dev,
        enum pm_device_action action)
{
    int ret = 0;

    switch (action) {
    case PM_DEVICE_ACTION_RESUME:
        ret = ht1632c_exit_sleep(dev);
        break;
    case PM_DEVICE_ACTION_SUSPEND:
        ret = ht1632c_enter_sleep(dev);
        break;
    default:
        ret = -ENOTSUP;
        break;
    }

    return ret;
}
#endif /* CONFIG_PM_DEVICE */


/**
 * @brief Inits the display driver
 *
 * @param dev Pointer to device structure
 *
 * @retval 0 on success else negative errno code.
 */
static int ht1632c_init(const struct device *dev)
{
    struct ht1632c_data *data = (struct ht1632c_data *)dev->data;
    const struct ht1632c_config *config = (struct ht1632c_config *)dev->config;
    uint8_t commons_command;

    printk("Configuring HT1632C\n");

    printk("Ticks per second %u\n", sys_clock_hw_cycles_per_sec());

    printk("Delay %u\n", ht1632c_ns_to_sys_clock_hw_cycles(600));

    data->width = 32;
    data->height = 8;

    printk("Commons options (%u)\n", config->commons_options);

    //set delays for the 4-wire protocol
    data->delays->cs = ht1632c_ns_to_sys_clock_hw_cycles(400);
    data->delays->clk = ht1632c_ns_to_sys_clock_hw_cycles(500);
    data->delays->su = ht1632c_ns_to_sys_clock_hw_cycles(250);
    data->delays->h = ht1632c_ns_to_sys_clock_hw_cycles(250);
    data->delays->su1 = ht1632c_ns_to_sys_clock_hw_cycles(300);
    data->delays->h1 = ht1632c_ns_to_sys_clock_hw_cycles(200);

    printk("Delay CS %u\n", data->delays->cs);
    printk("Delay CLK %u\n", data->delays->clk);
    printk("Delay SU %u\n", data->delays->su);
    printk("Delay H %u\n", data->delays->h);
    printk("Delay SU1 %u\n", data->delays->su1);
    printk("Delay H1 %u\n", data->delays->h1);


    if (config->cs_gpio.port != NULL) {
        if (!device_is_ready(config->cs_gpio.port)) {
            LOG_ERR("CS GPIO device not ready");
            return -ENODEV;
        }

        if (gpio_pin_configure_dt(&config->cs_gpio, GPIO_OUTPUT_HIGH | GPIO_ACTIVE_LOW)) {
            LOG_ERR("Couldn't configure CS pin");
            return -EIO;
        }
    }

    if (config->wr_gpio.port != NULL) {
        if (!device_is_ready(config->wr_gpio.port)) {
            LOG_ERR("WR GPIO device not ready");
            return -ENODEV;
        }

        if (gpio_pin_configure_dt(&config->wr_gpio, GPIO_OUTPUT_HIGH | GPIO_ACTIVE_LOW)) {
            LOG_ERR("Couldn't configure WR pin");
            return -EIO;
        }
    }

    if (config->data_gpio.port != NULL) {
        if (!device_is_ready(config->data_gpio.port)) {
            LOG_ERR("DATA GPIO device not ready");
            return -ENODEV;
        }

        if (gpio_pin_configure_dt(&config->data_gpio, GPIO_OUTPUT_LOW | GPIO_ACTIVE_HIGH)) {
            LOG_ERR("Couldn't configure DATA pin");
            return -EIO;
        }
    }

    printk("%s: device, GPIO pin %u is ready\n", dev->name, config->cs_gpio.pin);
    printk("%s: device, GPIO pin %u is ready\n", dev->name, config->wr_gpio.pin);
    printk("%s: device, GPIO pin %u is ready\n", dev->name, config->data_gpio.pin);

    printk("HT1632C sending init commands\n");

    ht1632c_write_command(dev, HT1632_SYS_ON);
    ht1632c_write_command(dev, HT1632_LED_ON);

    //HT1632C can be configured in a 32x8 or 24x16 pattern  
    //and common pad N-MOS open drain output  
    //or P-MOS open drain output  LED driver 
    switch(config->commons_options) {
        case 0x01:
            commons_command = HT1632_COM_01;
            data->width = 24;
            data->height = 16;
            break;
        case 0x10:
            commons_command = HT1632_COM_10;
            data->width = 32;
            data->height = 8;
            break;
        case 0x11:
            commons_command = HT1632_COM_11;
            data->width = 24;
            data->height = 16;
            break;
        case 0x00:
        default :
            commons_command = HT1632_COM_00;
            data->width = 32;
            data->height = 8;
    }

    printk("HT1632C commons command %u\n", commons_command);
    ht1632c_write_command(dev, commons_command);

#ifdef CONFIG_PM_DEVICE
    data->pm_state = PM_DEVICE_STATE_ACTIVE;
#endif
    
    return 0;
}


static const struct display_driver_api ht1632c_api = {
    .blanking_on = ht1632c_blanking_on,
    .blanking_off = ht1632c_blanking_off,
    .write = ht1632c_write,
    .read = ht1632c_read,
    .get_framebuffer = ht1632c_get_framebuffer,
    .set_brightness = ht1632c_set_brightness,
    .set_contrast = ht1632c_set_contrast,
    .get_capabilities = ht1632c_get_capabilities,
    .set_pixel_format = ht1632c_set_pixel_format,
};

static struct ht1632c_delays ht1632c_delays;


#define HT1632C_INIT(inst)                                       \
static struct ht1632c_config ht1632c_config_ ## inst = {         \
    .cs_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, cs_gpios, {}),     \
    .wr_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, wr_gpios, {}),     \
    .data_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, data_gpios, {}), \
    .commons_options = DT_INST_PROP(inst, commons_options)       \
};                                                               \
                                                                 \
static struct ht1632c_data ht1632c_data_ ## inst = {             \
    .delays = &ht1632c_delays                                    \
};                                                               \
                                                                 \
PM_DEVICE_DT_INST_DEFINE(inst, ht1632c_pm_action);               \
                                                                 \
DEVICE_DT_INST_DEFINE(inst, &ht1632c_init,                       \
    PM_DEVICE_DT_INST_REF(inst), &ht1632c_data_ ## inst,         \
    &ht1632c_config_ ## inst, APPLICATION,                       \
    CONFIG_APPLICATION_INIT_PRIORITY, &ht1632c_api);             

DT_INST_FOREACH_STATUS_OKAY(HT1632C_INIT)
