/*
 * Copyright (c) 2022 Farit N
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

#include "app_version.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/display.h>

#include <sys/printk.h>

#include <pm/device.h>

#include <drivers/sensor.h>
#include <drivers/i2c.h>

#include <drivers/eeprom.h>

#include <driver_rtc.h>

#include <ClockAlarm.h>
#include <ClockBackgroundLight.h>
#include <ClockButtons.h>
#include <ClockDisplay.h>
#include <ClockLightSensor.h>
#include <ClockSettings.h>
#include <ClockTemperature.h>
#include <ClockTime.h>
#include <ClockTimezone.h>
#include <ClockWatchdog.h>

K_THREAD_STACK_DEFINE(displayStackArea, 2048);
K_THREAD_STACK_DEFINE(buttonsStackArea, 4096);
K_THREAD_STACK_DEFINE(lightSensorStackArea, 2048);
K_THREAD_STACK_DEFINE(alarmStackArea, 2048);
K_THREAD_STACK_DEFINE(backgroundLightStackArea, 2048);


static k_tid_t displayThreadId;

/**
 * Processes buttons
 */
void processButtons(void *clockSettings, void *clockTime, void *clockDisplay)
{
    printk("In processButtons\n");

    printk("in buttons, threadId: %lu, currentThreadId: %lu\n", (unsigned long)displayThreadId, (unsigned long)k_current_get());

    ClockButtons clockButtons((ClockSettings*)clockSettings, (ClockTime*)clockTime, (ClockDisplay*)clockDisplay);

    clockButtons.processButtonActions();
}


/**
 * Sets the brightness of the display depending on the light level
 */
void adjustBrightness(void *clockSettings, void *clockDisplayInput, void*)
{
    //use this thread as watchdog
    ClockWatchdog watchdog;

    ClockLightSensor lightSensor;
    ClockDisplay *clockDisplay = (ClockDisplay*)clockDisplayInput;

    while(1) {
        uint8_t brightness = lightSensor.getBrightness();

        clockDisplay->setBrightness(brightness);

        //feed the watchdog, the thread runs well
        watchdog.feed();

        printk("Sleeping in adjustBrightness\n");
        //it must be smaller than the Watchdog window.max value
        k_msleep(18000);
    }

    return;
}

/**
 * Process an alarm
 */
void processAlarm(void *clockSettings, void *clockTime, void*)
{
    ClockAlarm alarm((ClockSettings*)clockSettings, (ClockTime*)clockTime);

    alarm.process();

    return;
}

/**
 * Process the background light change
 */
void processBackgroundLight(void *clockDisplay, void*, void*)
{
    ClockBackgroundLight backgroundLight((ClockDisplay*)clockDisplay);

    backgroundLight.process();

    return;
}


void displayTime(void*, void*, void*)
{
    ClockSettings clockSettings;

    ClockTime clockTime(&clockSettings);

    ClockTemperature clockTemperature;

    ClockDisplay clockDisplay(&clockSettings, &clockTime, &clockTemperature);
    clockDisplay.setThreadId(k_current_get());


    printk("in display, threadId: %lu, currentThreadId: %lu\n", (unsigned long)displayThreadId, (unsigned long)k_current_get());


    //creates a thread with the function processButtons
    struct k_thread buttonsThreadData;

    k_thread_create(&buttonsThreadData, buttonsStackArea,
        K_THREAD_STACK_SIZEOF(buttonsStackArea), processButtons, &clockSettings, &clockTime, &clockDisplay, -1, 0, K_NO_WAIT);

    //creates a thread with the function adjustBrightness
    struct k_thread lightSensorThreadData;

    k_thread_create(&lightSensorThreadData, lightSensorStackArea,
        K_THREAD_STACK_SIZEOF(lightSensorStackArea), adjustBrightness, &clockSettings, &clockDisplay, NULL, 6, 0, K_MSEC(100));

    //creates a thread with the function processBackgroundLight
    struct k_thread backgroundLightThreadData;

    k_thread_create(&backgroundLightThreadData, backgroundLightStackArea,
        K_THREAD_STACK_SIZEOF(backgroundLightStackArea), processBackgroundLight, &clockDisplay, NULL, NULL, 7, 0, K_MSEC(100));

    //creates a thread with the function processAlarm
    struct k_thread alarmThreadData;

    k_thread_create(&alarmThreadData, alarmStackArea,
        K_THREAD_STACK_SIZEOF(alarmStackArea), processAlarm, &clockSettings, &clockTime, NULL, 3, 0, K_NO_WAIT);

    //so the lightSensor process finish
    k_msleep(300);

    while(1) {

        clockTime.getRtcTime();

        printk("Time: %.2d:%.2d:%.2d", clockTime.getHour(), clockTime.getMinute(), clockTime.getSecond());        
        printk(", Date: %.2d-%.2d-%.2d, Weekday: %.2d\n", clockTime.getYear(), clockTime.getMonth(), clockTime.getDay(), clockTime.getWeekday());
        
        clockDisplay.show();

        k_msleep(clockDisplay.getSleepTime());
    }

    return;
}

void main(void)
{
    //creates a thread with the function processButtons
    struct k_thread displayThreadData;

    displayThreadId = k_thread_create(&displayThreadData, displayStackArea,
        K_THREAD_STACK_SIZEOF(displayStackArea), displayTime, NULL, NULL, NULL, 1, 0, K_NO_WAIT);

    printk("in main, threadId: %zu\n", (size_t)displayThreadId);

    return;
}
