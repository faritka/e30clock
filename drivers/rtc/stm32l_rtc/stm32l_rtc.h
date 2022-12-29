/*
 * Copyright (c) 2021 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef STM32L_RTC_DRIVER_H__
#define STM32L_RTC_DRIVER_H__

#define DT_DRV_COMPAT st_stm32lrtc

#include <zephyr/kernel.h>
#include <time.h>
#include <zephyr/device.h>
#include <zephyr/pm/device.h>
#include <zephyr/sys/printk.h>
#include <soc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(stm32lrtc, CONFIG_KERNEL_LOG_LEVEL);

#include <stm32_ll_bus.h>
#include <stm32_ll_pwr.h>
#include <stm32_ll_rcc.h>
#include <stm32_ll_rtc.h>

#include <driver_rtc.h>
#include "stm32_hsem.h"


// Defines related to Clock configuration
// LSE as RTC clock
#define RTC_ASYNCH_PREDIV  0x7F 
#define RTC_SYNCH_PREDIV   0x00FF 

// Define used to indicate date/time updated
#define RTC_BKP_DATE_TIME_UPDATED ((uint32_t)0x4321)

#if defined(CONFIG_SOC_SERIES_STM32L4X)
#define RTC_EXTI_LINE	LL_EXTI_LINE_18
#elif defined(CONFIG_SOC_SERIES_STM32G0X)
#define RTC_EXTI_LINE	LL_EXTI_LINE_19
#elif defined(CONFIG_SOC_SERIES_STM32F4X) \
	|| defined(CONFIG_SOC_SERIES_STM32F0X) \
	|| defined(CONFIG_SOC_SERIES_STM32F2X) \
	|| defined(CONFIG_SOC_SERIES_STM32F3X) \
	|| defined(CONFIG_SOC_SERIES_STM32F7X) \
	|| defined(CONFIG_SOC_SERIES_STM32WBX) \
	|| defined(CONFIG_SOC_SERIES_STM32G4X) \
	|| defined(CONFIG_SOC_SERIES_STM32L0X) \
	|| defined(CONFIG_SOC_SERIES_STM32L1X) \
	|| defined(CONFIG_SOC_SERIES_STM32H7X) \
	|| defined(CONFIG_SOC_SERIES_STM32WLX)
#define RTC_EXTI_LINE	LL_EXTI_LINE_17
#endif


/** @brief Driver config data */
struct stm32l_rtc_config {
};

/** @brief Driver instance data */
struct stm32l_rtc_data {
    rtc_alarm_callback_t callback;
    void *user_data;
#ifdef CONFIG_PM_DEVICE
    uint32_t pm_state;
#endif
};

static void stm32_rtc_irq_config(const struct device *dev);

#endif
