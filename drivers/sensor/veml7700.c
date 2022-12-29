/*
 * Copyright (c) 2021 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT vishay_veml7700

#include "veml7700.h"
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>

LOG_MODULE_REGISTER(veml7700, CONFIG_SENSOR_LOG_LEVEL);

int veml7700_read(const struct device *dev, uint8_t reg, uint16_t *out)
{
    const struct veml7700_config *config = dev->config;
    uint8_t buff[2] = { 0 };
    int ret = 0;

    ret = i2c_write_read_dt(&config->i2c, &reg, sizeof(reg), buff, sizeof(buff));

    if (!ret) {
        *out = sys_get_le16(buff);
    }

    return ret;
}

int veml7700_write(const struct device *dev, uint8_t reg, uint16_t value)
{
    const struct veml7700_config *config = dev->config;
    struct i2c_msg msg;
    int ret;
    uint8_t buff[3];

    sys_put_le16(value, &buff[1]);

    buff[0] = reg;

    msg.buf = buff;
    msg.flags = 0;
    msg.len = sizeof(buff);

    ret = i2c_transfer_dt(&config->i2c, &msg, 1);

    if (ret < 0) {
        LOG_ERR("write block failed");
        return ret;
    }

    return 0;
}

static int veml7700_sample_fetch(const struct device *dev,
            enum sensor_channel chan)
{
    struct veml7700_data *data = dev->data;
    int ret = 0;

    __ASSERT_NO_MSG((chan == SENSOR_CHAN_ALL) ||
        (chan == SENSOR_CHAN_LIGHT));


    if (chan == SENSOR_CHAN_ALL || chan == SENSOR_CHAN_LIGHT) {
        ret = veml7700_read(dev, VEML7700_REG_ALS_DATA, &data->light);

    printk("Light level: %u\n", data->light);
        if (ret < 0) {
            LOG_ERR("Could not fetch ambient light");
        }
    }

    return ret;
}



static int veml7700_channel_get(const struct device *dev,
                enum sensor_channel chan,
                struct sensor_value *val)
{
    struct veml7700_data *data = dev->data;
    int ret = 0;

    switch (chan) {
        case SENSOR_CHAN_LIGHT:
            val->val1 = data->light;
            val->val2 = 0;
        break;

    default:
        ret = -ENOTSUP;
    }

    return ret;
}

#ifdef CONFIG_PM_DEVICE
static int veml7700_pm_action(const struct device *dev,
          enum pm_device_action action)
{
    int ret = 0;
    uint16_t ps_conf;

    ret = veml7700_read(dev, VEML7700_REG_PSM, &ps_conf);
    if (ret < 0) {
        return ret;
    }

    uint16_t als_conf;

    ret = veml7700_read(dev, VEML7700_REG_CONF, &als_conf);
    if (ret < 0) {
        return ret;
    }

    switch (action) {
    case PM_DEVICE_ACTION_RESUME:
        /* Clear als shutdown */
        als_conf &= ~VEML7700_ALS_SD_MASK;

        ret = veml7700_write(dev, VEML7700_REG_CONF, als_conf);
        if (ret < 0) {
            return ret;
        }

        break;
    case PM_DEVICE_ACTION_SUSPEND:
        /* Set als shutdown bit 0 */
        als_conf |= VEML7700_ALS_SD_MASK;

        ret = veml7700_write(dev, VEML7700_REG_CONF, als_conf);
        if (ret < 0) {
            return ret;
        }

        break;
    default:
        return -ENOTSUP;
    }

    return ret;
}
#endif /* CONFIG_PM_DEVICE */


static int veml7700_init(const struct device *dev)
{
    const struct veml7700_config *config = dev->config;
    uint16_t conf = 0;

    uint16_t tmp, tmp1 = 0;

    k_msleep(20);

    /* Get the I2C device */
    //data->i2c = device_get_binding(config->i2c_name);
    if (!device_is_ready(config->i2c.bus)) {
        LOG_ERR("Bus device is not ready");
        return -ENODEV;
    }


    //if (veml7700_read(dev, VEML7700_REG_CONF, &conf)) {
    //    LOG_ERR("Could not read config");
    //    return -EIO;
    //}

    tmp = conf;

    printk("Config VEML7700: %u\n", tmp);

    printk("ALS gain: %u\n", config->als_gain);
    printk("ALS it: %u\n", config->als_it);
    printk("ALS pers: %u\n", config->als_pers);
    printk("PSM: %u\n", config->psm);

    // Set ALS gain
    conf |= config->als_gain << VEML7700_ALS_GAIN_POS;

    tmp = conf;
    printk("Config after gain: %u\n", tmp);
    
    // Set ALS integration time
    conf |= config->als_it << VEML7700_ALS_IT_POS;
    tmp = conf;

    printk("Config after integration: %u\n", tmp);

    // Set ALS persistence protect number 
    conf |= config->als_pers << VEML7700_ALS_PERS_POS;

    tmp = conf;

    printk("Config after persistance: %u\n", tmp);

    // Clear ALS shutdown
    conf &= ~VEML7700_ALS_SD_MASK;
    tmp1 = conf;

    printk("Config after shutdown clear: %u\n", tmp1);

    if (veml7700_write(dev, VEML7700_REG_CONF, conf)) {
        LOG_ERR("Could not write config");
        printk("Could not write config");
        return -EIO;
    }

    LOG_DBG("Init complete");
    printk("Init complete");

    return 0;
}

static const struct sensor_driver_api veml7700_driver_api = {
    .sample_fetch = veml7700_sample_fetch,
    .channel_get = veml7700_channel_get,
};

#define VEML7700_INIT(inst)                                        \
static const struct veml7700_config veml7700_config_ ## inst = {   \
    .i2c = I2C_DT_SPEC_INST_GET(inst),                             \
    .als_gain = DT_INST_PROP(inst, als_gain),                      \
    .als_it = DT_INST_PROP(inst, als_it),                          \
    .als_pers = DT_INST_PROP(inst, als_pers),                      \
    .psm = DT_INST_PROP(inst, psm),                                \
};                                                                 \
                                                                   \
static struct veml7700_data veml7700_data_ ## inst;                \
                                                                   \
PM_DEVICE_DT_INST_DEFINE(inst, veml7700_pm_action);                \
                                                                   \
DEVICE_DT_INST_DEFINE(inst, &veml7700_init,                        \
    PM_DEVICE_DT_INST_REF(inst), &veml7700_data_ ## inst,          \
    &veml7700_config_ ## inst,                                     \
    POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,                      \
    &veml7700_driver_api);

DT_INST_FOREACH_STATUS_OKAY(VEML7700_INIT)
