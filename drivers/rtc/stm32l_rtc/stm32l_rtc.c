/*
 * Copyright (c) 2021 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "stm32l_rtc.h"

#ifdef CONFIG_PM_DEVICE
/**
 * Turns on the chip from sleep
 * @param dev Pointer to data structure
 */
static int stm32l_rtc_exit_sleep(struct stm32l_rtc_data *data)
{
    k_sleep(K_MSEC(120));

    return 0;
}

/**
 * Turns off the chip into sleep
 * @param dev Pointer to data structure
 */
static int stm32l_rtc_enter_sleep(struct stm32l_rtc_data *data)
{
    return 0;
}
#endif

#ifdef CONFIG_PM_DEVICE
static int stm32l_rtc_pm_action(const struct device *dev,
        enum pm_device_action action)
{
    int ret = 0;
    struct stm32l_rtc_data *data = (struct stm32l_rtc_data *)dev->data;

    switch (action) {
    case PM_DEVICE_ACTION_RESUME:
        ret = stm32l_rtc_exit_sleep(data);
        break;
    case PM_DEVICE_ACTION_SUSPEND:
        ret = stm32l_rtc_enter_sleep(data);
        break;
    default:
        ret = -ENOTSUP;
        break;
    }

    return ret;
}
#endif /* CONFIG_PM_DEVICE */


/**
 * Starts the RTC
  * @param dev Pointer to device structure
 */
static int stm32l_rtc_start(const struct device *dev)
{
    ARG_UNUSED(dev);

    z_stm32_hsem_lock(CFG_HW_RCC_SEMID, HSEM_LOCK_DEFAULT_RETRY);
    //LL_RCC_EnableRTC();
    z_stm32_hsem_unlock(CFG_HW_RCC_SEMID);

    return 0;
}

/**
 * Stops the RTC
  * @param dev Pointer to device structure
 */
static int stm32l_rtc_stop(const struct device *dev)
{
    ARG_UNUSED(dev);

    z_stm32_hsem_lock(CFG_HW_RCC_SEMID, HSEM_LOCK_DEFAULT_RETRY);
    //LL_RCC_DisableRTC();
    z_stm32_hsem_unlock(CFG_HW_RCC_SEMID);

    return 0;
}

/**
 * Gets the RTC time
 * @param dev Pointer to device structure
 * @param tm Pointer to tm structure
 */
static int stm32l_rtc_get_time(const struct device *dev, struct tm *tm)
{
    ARG_UNUSED(dev);

    /* Note: need to convert in decimal value in using __LL_RTC_CONVERT_BCD2BIN helper macro */

    printk("rtc_get_time: %.2d:%.2d:%.2d", __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC)),
          __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC)),
          __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC)));
    printk(", Date: %.2d-%.2d-%.2d\n", __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetMonth(RTC)),
          __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetDay(RTC)),
          2000 + __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC)));


    tm->tm_sec = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));
    tm->tm_min = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
    tm->tm_hour = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));

    tm->tm_mday = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetDay(RTC));
    tm->tm_mon = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetMonth(RTC)) - 1;
    tm->tm_year = 100 + __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC));

    tm->tm_wday = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetWeekDay(RTC));

    return 0;
}

/**
 * Sets the RTC time
 * @param dev Pointer to device structure
 * @param tm Pointer to tm structure
 */
static int stm32l_rtc_set_time(const struct device *dev, struct tm *tm)
{
    ARG_UNUSED(dev);

    LL_RTC_TimeTypeDef RTC_TimeStruct = {0};
    LL_RTC_DateTypeDef RTC_DateStruct = {0};

    z_stm32_hsem_lock(CFG_HW_RCC_SEMID, HSEM_LOCK_DEFAULT_RETRY);

    printk("In rtc_set_time, year: %.2d\n", tm->tm_year);

    LL_RTC_DisableWriteProtection(RTC);

    // Initialize RTC and set the Time and Date
    RTC_TimeStruct.Hours = __LL_RTC_CONVERT_BIN2BCD(tm->tm_hour);
    RTC_TimeStruct.Minutes = __LL_RTC_CONVERT_BIN2BCD(tm->tm_min);
    RTC_TimeStruct.Seconds = __LL_RTC_CONVERT_BIN2BCD(tm->tm_sec);
    LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_TimeStruct);

    // Sunday is the 7th day
    RTC_DateStruct.WeekDay = __LL_RTC_CONVERT_BIN2BCD((tm->tm_wday == 0) ? 7 : tm->tm_wday);
    RTC_DateStruct.Month = __LL_RTC_CONVERT_BIN2BCD(tm->tm_mon + 1);
    RTC_DateStruct.Day = __LL_RTC_CONVERT_BIN2BCD(tm->tm_mday);
    RTC_DateStruct.Year = __LL_RTC_CONVERT_BIN2BCD(tm->tm_year - 100);
    LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_DateStruct);

    LL_RTC_EnableWriteProtection(RTC);

    printk("In rtc_settime Time: %.2d:%.2d:%.2d", tm->tm_hour, tm->tm_min, tm->tm_sec);

    printk(", Date: %.2d-%.2d-%.2d, Weekday: %.2d\n", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday);

    z_stm32_hsem_unlock(CFG_HW_RCC_SEMID);

    printk("Exited rtc_settime\n");

    return 0;
}

/**
 * @brief Inits the STM32 RTC
 *
 * @param dev Pointer to device structure
 *
 * @retval 0 on success else negative errno code.
 */
static int stm32l_rtc_init(const struct device *dev)
{
#ifdef CONFIG_PM_DEVICE
    struct stm32l_rtc_data *data = (struct stm32l_rtc_data *)dev->data;
    //const struct stm32l_rtc_config *config = (struct stm32l_rtc_config *)dev->config;
#endif

    LL_RTC_InitTypeDef RTC_InitStruct = {0};
    LL_RTC_TimeTypeDef RTC_TimeStruct = {0};
    LL_RTC_DateTypeDef RTC_DateStruct = {0};

    printk("Configuring STM32 RTC\n");

    z_stm32_hsem_lock(CFG_HW_RCC_SEMID, HSEM_LOCK_DEFAULT_RETRY);

    printk("GRP1\n");

    if (LL_PWR_IsEnabledBkUpAccess () != 1U) {
        // Enable write access to Backup domain
        LL_PWR_EnableBkUpAccess();
        while (LL_PWR_IsEnabledBkUpAccess () == 0U) {}
    }

    printk("BkUpAccess\n");

    //LL_RCC_ForceBackupDomainReset();
    //LL_RCC_ReleaseBackupDomainReset();

    // For different crystals. 6pF - low strength, for 12.5pF - high strength
    LL_RCC_LSE_SetDriveCapability(CONFIG_STM32L_RTC_LSE_DRIVE_STRENGTH);
    LL_RCC_LSE_Enable();

    printk("LSE_ENABLE\n");

    // Wait till LSE is ready
    while(LL_RCC_LSE_IsReady() != 1) {}

    printk("LSE_isReady\n");

    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    printk("SetTCCClock\n");

    //it has never been configured
    if (LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR1) != RTC_BKP_DATE_TIME_UPDATED) {
        printk("RTC has not been configured");

        // Peripheral clock enable
        LL_RCC_EnableRTC();

        LL_RTC_DisableWriteProtection(RTC);
        LL_RTC_EnableInitMode(RTC);

        // Initialize RTC and set the Time and Date
        RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
        RTC_InitStruct.AsynchPrescaler = 127;
        RTC_InitStruct.SynchPrescaler = 255;
        LL_RTC_Init(RTC, &RTC_InitStruct);

        printk("After LL_RTC_Init\n");

        // Initialize RTC and set the Time and Date
        RTC_TimeStruct.Hours = 0x00;
        RTC_TimeStruct.Minutes = 0x00;
        RTC_TimeStruct.Seconds = 0x00;
        LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_TimeStruct);

        printk("After LL_RTC_TIME_Init\n");

        RTC_DateStruct.WeekDay = LL_RTC_WEEKDAY_SATURDAY;
        RTC_DateStruct.Month = LL_RTC_MONTH_JANUARY;
        RTC_DateStruct.Day = 0x01;
        RTC_DateStruct.Year = 0x00;
        LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_DateStruct);

        printk("After LL_RTC_DATE_Init\n");

        LL_RTC_DisableInitMode(RTC);
        LL_RTC_EnableWriteProtection(RTC);

        //Write the indicator that the RTC has been configured
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR1, RTC_BKP_DATE_TIME_UPDATED);
    }

    z_stm32_hsem_unlock(CFG_HW_RCC_SEMID);

    printk("After hsem unlock\n");

#ifdef CONFIG_PM_DEVICE
    data->pm_state = PM_DEVICE_STATE_ACTIVE;
#endif

#if defined(CONFIG_SOC_SERIES_STM32H7X) && defined(CONFIG_CPU_CORTEX_M4)
    LL_C2_EXTI_EnableIT_0_31(RTC_EXTI_LINE);
#else
    LL_EXTI_EnableIT_0_31(RTC_EXTI_LINE);
#endif
    LL_EXTI_EnableRisingTrig_0_31(RTC_EXTI_LINE);

    stm32_rtc_irq_config(dev);

    return 0;
}

static int stm32_rtc_set_alarm(const struct device *dev, 
    const struct rtc_alarm_cfg *alarm_cfg, struct tm *tm, uint32_t mask)
{
    struct stm32l_rtc_data *data = (struct stm32l_rtc_data *)dev->data;

    LL_RTC_AlarmTypeDef rtc_alarm;

    data->callback = alarm_cfg->callback;
    data->user_data = alarm_cfg->user_data;

    /* Apply ALARM_A */
    rtc_alarm.AlarmTime.TimeFormat = LL_RTC_TIME_FORMAT_AM_OR_24;
    rtc_alarm.AlarmTime.Hours = tm->tm_hour;
    rtc_alarm.AlarmTime.Minutes = tm->tm_min;
    rtc_alarm.AlarmTime.Seconds = tm->tm_sec;

    rtc_alarm.AlarmMask = mask;
    rtc_alarm.AlarmDateWeekDaySel = LL_RTC_ALMA_DATEWEEKDAYSEL_DATE;
    rtc_alarm.AlarmDateWeekDay = (tm->tm_wday == 0) ? 7 : tm->tm_wday;

    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_ALMA_Disable(RTC);
    LL_RTC_EnableWriteProtection(RTC);

    if (LL_RTC_ALMA_Init(RTC, LL_RTC_FORMAT_BIN, &rtc_alarm) != SUCCESS) {
        return -EIO;
    }

    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_ALMA_Enable(RTC);
    LL_RTC_ClearFlag_ALRA(RTC);
    LL_RTC_EnableIT_ALRA(RTC);
    LL_RTC_EnableWriteProtection(RTC);

    return 0;
}


static int stm32_rtc_cancel_alarm(const struct device *dev)
{
    struct stm32l_rtc_data *data = (struct stm32l_rtc_data *)dev->data;

    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_ClearFlag_ALRA(RTC);
    LL_RTC_DisableIT_ALRA(RTC);
    LL_RTC_ALMA_Disable(RTC);
    LL_RTC_EnableWriteProtection(RTC);

    data->callback = NULL;

    return 0;
}

void stm32_rtc_isr(const struct device *dev)
{
    struct stm32l_rtc_data *data = (struct stm32l_rtc_data *)dev->data;

    rtc_alarm_callback_t alarm_callback = data->callback;

    if (LL_RTC_IsActiveFlag_ALRA(RTC) != 0) {

        LL_RTC_DisableWriteProtection(RTC);
        LL_RTC_ClearFlag_ALRA(RTC);
        //LL_RTC_DisableIT_ALRA(RTC);
        //LL_RTC_ALMA_Disable(RTC);
        LL_RTC_EnableWriteProtection(RTC);

        if (alarm_callback != NULL) {
            alarm_callback(dev, data->user_data);
        }
    }

    printk("After alarm_callback\n");

#if defined(CONFIG_SOC_SERIES_STM32H7X) && defined(CONFIG_CPU_CORTEX_M4)
    LL_C2_EXTI_ClearFlag_0_31(RTC_EXTI_LINE);
#elif defined(CONFIG_SOC_SERIES_STM32G0X)
    LL_EXTI_ClearRisingFlag_0_31(RTC_EXTI_LINE);
#else
    LL_EXTI_ClearFlag_0_31(RTC_EXTI_LINE);
#endif
}



static const struct rtc_driver_api stm32l_rtc_api = {
    .start = stm32l_rtc_start,
    .stop = stm32l_rtc_stop,
    .get_time = stm32l_rtc_get_time,
    .set_time = stm32l_rtc_set_time,
    .set_alarm = stm32_rtc_set_alarm,
    .cancel_alarm = stm32_rtc_cancel_alarm,
};

static struct stm32l_rtc_data stm32l_rtc_data = {};

static struct stm32l_rtc_config stm32l_rtc_config = {};

PM_DEVICE_DT_INST_DEFINE(0, stm32l_rtc_pm_action);

DEVICE_DT_INST_DEFINE(0, &stm32l_rtc_init,
          PM_DEVICE_DT_INST_REF(0), &stm32l_rtc_data, &stm32l_rtc_config, APPLICATION,
          CONFIG_APPLICATION_INIT_PRIORITY, &stm32l_rtc_api);

static void stm32_rtc_irq_config(const struct device *dev)
{
    IRQ_CONNECT(DT_INST_IRQN(0),
            DT_INST_IRQ(0, priority),
            stm32_rtc_isr, DEVICE_DT_INST_GET(0), 0);
    irq_enable(DT_INST_IRQN(0));
}
