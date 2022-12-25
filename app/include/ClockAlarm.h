/*
 * The class that handles an alarm by playing a signal
 * 
 */
#ifndef __CLOCK_ALARM_H
#define __CLOCK_ALARM_H

#include <sys/printk.h>

#include <device.h>

#include <ClockGong.h>
#include <ClockSettings.h>
#include <ClockTime.h>

class ClockAlarm
{
    public:
        ClockAlarm(ClockSettings *clockSettings, ClockTime *clockTime);

        void process();

    private:
        ClockSettings *clockSettings;

        ClockTime *clockTime;

};

#endif

