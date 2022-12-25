/*
 * Copyright (c) 2022 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef RV3032_DRIVER_H__
#define RV3032_DRIVER_H__

#define DT_DRV_COMPAT microcrystal_rv3032

#include <zephyr.h>
#include <time.h>
#include <device.h>
#include <drivers/i2c.h>
#include <drivers/gpio.h>
#include <pm/device.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <soc.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(rv3032, CONFIG_KERNEL_LOG_LEVEL);

#include <driver_rtc.h>

#define RV3032_I2C_ADDRESS           0x51

#define RV3032_HUNDREDTHS            0x00
#define RV3032_SECONDS               0x01
#define RV3032_MINUTES               0x02
#define RV3032_HOURS                 0x03
#define RV3032_WEEKDAYS              0x04
#define RV3032_DATE                  0x05
#define RV3032_MONTHS                0x06
#define RV3032_YEARS                 0x07

#define RV3032_MINUTES_ALARM         0x08
#define RV3032_HOURS_ALARM           0x09
#define RV3032_DATE_ALARM            0x0A
#define RV3032_TIMER_0               0x0B
#define RV3032_TIMER_1               0x0C
#define RV3032_STATUS                0x0D
#define RV3032_TEMP_LSB              0x0E
#define RV3032_TEMP_MSB              0x0F
#define RV3032_CONTROL1              0x10
#define RV3032_CONTROL2              0x11
#define RV3032_CONTROL3              0x12
#define RV3032_TS_CONTROL            0x13
#define RV3032_CLOCK_INT_MASK        0x14
#define RV3032_EVI_CONTROL           0x15

#define RV3032_EEADDR                0x3D
#define RV3032_EEDATA                0x3E
#define RV3032_EECMD                 0x3F

#define RV3032_EEPROM_PMU            0xC0
#define RV3032_EEPROM_OFFSET         0xC1
#define RV3032_EEPROM_CLKOUT1        0xC2
#define RV3032_EEPROM_CLKOUT2        0xC3

#define RV3032_IRQ_THREAD_STACK_SIZE 2048

/** @brief Driver config data */
struct rv3032_config {
    char *i2c_name;
    //the interrupt pin
    struct gpio_dt_spec int_gpio;
    //CLKOUT enable bit
    uint8_t nclke;
    //backup power switchover mode
    uint8_t bsm;
    //trickle charger mode
    uint8_t tcm;
    //trickle charger series resistor
    uint8_t tcr;
};

/** @brief Driver instance data */
struct rv3032_data {
    const struct device *i2c;
    //the lock while accessing I2C
    struct k_mutex lock;
    //called when there is an interrupt on the interrupt gpio pin
    struct gpio_callback int_gpio_cb;
    //the callback function from the higher level
    rtc_alarm_callback_t callback;
    void *user_data;
#ifdef CONFIG_PM_DEVICE
    uint32_t pm_state;
#endif
    //the thread processes interrupts
    struct k_thread irq_thread;
    struct k_sem irq_sem;
    K_KERNEL_STACK_MEMBER(irq_thread_stack, RV3032_IRQ_THREAD_STACK_SIZE);
};

static int rv3032_irq_config(const struct device *dev);

#endif
