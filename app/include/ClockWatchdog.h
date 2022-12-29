/*
 * The class that runs a watchdog
 * 
 */
#ifndef __CLOCK_WATCHDOG_H
#define __CLOCK_WATCHDOG_H

#include <zephyr/sys/printk.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/watchdog.h>

class ClockWatchdog
{
    public:
        ClockWatchdog();

        void feed();

    private:
        const struct device *watchdog;

        int watchdogChannelId;

        int init();

};

#endif

