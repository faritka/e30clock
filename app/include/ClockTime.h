/*
 * The class for time functions
 * 
 */
#ifndef __CLOCK_TIME_H
#define __CLOCK_TIME_H

#include <time.h>
#include <driver_rtc.h>
#include <stm32_ll_rtc.h>

#include <zephyr/sys/printk.h>

#include <ClockSettings.h>
#include <ClockTimeLib.h>
#include <ClockTimezone.h>


class ClockTime 
{
    public:
        ClockTime(ClockSettings *clockSettings);

        //the semaphore to tell the ClockAlarm thread that an alarm was triggered
        static struct k_sem alarmSemaphore;

        /**
         * Gets the time from the RTC device
         */
        int64_t getRtcTime();

        /**
         * Sets the time into the RTC device
         */
        int setRtcTime();

        void setHour(uint8_t hour)
        {
            tm.tm_hour = hour;
        }

        void setMinute(uint8_t minute)
        {
            tm.tm_min = minute;
        }

        void setSecond(uint8_t second)
        {
            tm.tm_sec = second;
        }

        void setDay(uint8_t day)
        {
            tm.tm_mday = day;
        }

        void setMonth(uint8_t month)
        {
            tm.tm_mon = month - 1;
        }

        void setYear(uint32_t year)
        {
            tm.tm_year = year - 1900;
        }

        uint8_t getHour() 
        {
            return tm.tm_hour;
        }

        // the hour for the given time in 12 hour format
        uint8_t getHourFormat12() 
        {
            // 12 midnight
            if (tm.tm_hour == 0) {
                return 12; 
            } else if (tm.tm_hour > 12) {
                return (tm.tm_hour - 12);
            } else {
                return tm.tm_hour;
            }
        }

        // returns true if AM
        uint8_t isAM() 
        { 
            return !isPM(); 
        }

        // returns true if PM
        uint8_t isPM() 
        {
            return (getHour() >= 12); 
        }

        uint8_t getMinute() 
        {
            return tm.tm_min; 
        }

        uint8_t getSecond() 
        {
            return tm.tm_sec; 
        }

        uint8_t getDay()
        {
            return tm.tm_mday;
        }

        uint8_t getWeekday() 
        {
            return tm.tm_wday;
        }

        /**
         * Returns the weekday name by its weekday number
         */
        inline const char *getWeekdayName()
        {
            return (ClockTime::weekdayNames[this->getWeekday()]);
        }
   
        uint8_t getMonth()
        {
            return (tm.tm_mon + 1); 
        }

        const char *getMonthName()
        {        
            return (ClockTime::monthNames[this->getMonth() - 1]);
        }


        uint32_t getYear() 
        {
            return (tm.tm_year + 1900); 
        }

        /** 
         * Returns the frequency correction offset from the RTC
         */
        int8_t getCorrectionOffset();

        /**
         * Sets the frequency correction offset to the RTC
         * @param int8_t The offset is from -32 to +31
         */
        void setCorrectionOffset(int8_t offset);

        ClockTimezone *getTimezone()
        {
            return &timezone;
        }

        /**
         * Gets the number of days in the current month: 30, 31, 28, 29
         */
        uint8_t getDaysInMonth();

        //Sets the alarm interrupt
        void setAlarmInterrupt();


        /**
         * Checks if the local standard to DST change time is within 1 hour from the supplied time
         * It's necessary if we need to set new time on a DST day.
         */
        inline bool isStartDstWithinOneHourLocal(int64_t currentTime)
        {
            printk("isStartDstWithinOneHour %lld\n", currentTime);

            printk("timezone.getStartDst: %lld\n", timezone.getStartDst());

            printk("timezone.getStartDst + 1 hour: %lld\n", timezone.getStartDst() + 1 * 60 * 60);

            return (((currentTime >= timezone.getStartDst()) && (currentTime <= (timezone.getStartDst() + 1 * 60 * 60))) ? true : false);
        }

        /**
         * Checks if the local DST to Standard change time is within 1 hour from the supplied time
         * It's necessary if we need to set new time on a DST day.
         */
        inline bool isStartStdWithinOneHourLocal(int64_t currentTime)
        {
            return (((currentTime >= timezone.getStartStd()) && (currentTime <= (timezone.getStartStd() + 1 * 60 * 60))) ? true : false);
        }

    private:
        //the RTC device
        const struct device *rtc;

        struct tm tm;
        struct tm tmUtc;

        //the weekday names
        static const char weekdayNames[8][4];

        //the month names
        static const char monthNames[12][4];

        //the Settings object
        ClockSettings *clockSettings;

        //the Timezone object
        ClockTimezone timezone;
        
        //the mutex to limit simultaneous access to the RTC
        struct k_mutex mutexRtc;

        //the config for the alarm that includes the callback function
        static struct rtc_alarm_cfg alarmConf;

        //the function that is called when an interrupt is triggered
        static void alarmCallback(const struct device *dev, void *user_data);

};

#endif

