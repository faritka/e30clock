/*
 * The class that runs a watchdog
 * 
 */
#ifndef __CLOCK_WATCHDOG_H
#define __CLOCK_WATCHDOG_H

#include <sys/printk.h>

#include <device.h>
#include <devicetree.h>
#include <drivers/watchdog.h>

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

